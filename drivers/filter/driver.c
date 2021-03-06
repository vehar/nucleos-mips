/* Filter driver - lowest layer - disk driver management */

#include "inc.h"

/* Drivers. */
static struct {
  char *label;
  int minor;
  endpoint_t endpt;

  int problem;		/* one of BD_* */
  int error;		/* one of E*, only relevant if problem>0 */
  int retries;
  int kills;
} driver[2];

/* State variables. */
static endpoint_t self_ep;
static asynmsg_t amsgtable[2];

static int size_known = 0;
static u64_t disk_size;

static int problem_stats[BD_LAST] = { 0 };

/*===========================================================================*
 *				driver_open				     *
 *===========================================================================*/
static int driver_open(int which)
{
	/* Perform an open or close operation on the driver. This is
	 * unfinished code: we should never be doing a blocking kipc_module_call() to
	 * the driver.
	 */
	kipc_msg_t msg;
	cp_grant_id_t gid;
	struct partition part;
	sector_t sectors;
	int r;

	msg.m_type = DEV_OPEN;
	msg.DEVICE = driver[which].minor;
	msg.IO_ENDPT = self_ep;
	r = kipc_module_call(KIPC_SENDREC, 0, driver[which].endpt, &msg);

	if (r != 0) {
		/* Should we restart the driver now? */
		printk("Filter: driver_open: kipc_module_call returned %d\n", r);

		return RET_REDO;
	}

	if(msg.m_type != KCNR_TASK_REPLY || msg.REP_STATUS != 0) {
		printk("Filter: driver_open: kipc_module_call returned %d, %d\n",
			msg.m_type, msg.REP_STATUS);

		return RET_REDO;
	}

	/* Take the opportunity to retrieve the hard disk size. */
	gid = cpf_grant_direct(driver[which].endpt,
		(vir_bytes) &part, sizeof(part), CPF_WRITE);
	if(!GRANT_VALID(gid))
		panic(__FILE__, "invalid grant", gid);

	msg.m_type = DEV_IOCTL_S;
	msg.REQUEST = DIOCGETP;
	msg.DEVICE = driver[which].minor;
	msg.IO_ENDPT = self_ep;
	msg.IO_GRANT = (char *) gid;

	r = kipc_module_call(KIPC_SENDREC, 0, driver[which].endpt, &msg);

	cpf_revoke(gid);

	if (r != 0 || msg.m_type != KCNR_TASK_REPLY || msg.REP_STATUS != 0) {
		/* Not sure what to do here, either. */
		printk("Filter: ioctl(DIOCGETP) returned (%d, %d)\n", 
			r, msg.m_type);

		return RET_REDO;
	}

	if(!size_known) {
		disk_size = part.size;
		size_known = 1;
		sectors = div64u(disk_size, SECTOR_SIZE);
		if(cmp64(mul64u(sectors, SECTOR_SIZE), disk_size)) {
			printk("Filter: partition too large\n");

			return RET_REDO;
		}
#if DEBUG
		printk("Filter: partition size: 0x%s / %lu sectors\n",
			print64(disk_size), sectors);
#endif
	} else {
		if(cmp64(disk_size, part.size)) {
			printk("Filter: partition size mismatch (%s != %s)\n",
				print64(part.size), print64(disk_size));

			return RET_REDO;
		}
	}

	return 0;
}

/*===========================================================================*
 *				driver_close				     *
 *===========================================================================*/
static int driver_close(int which)
{
	kipc_msg_t msg;
	int r;

	msg.m_type = DEV_CLOSE;
	msg.DEVICE = driver[which].minor;
	msg.IO_ENDPT = self_ep;
	r = kipc_module_call(KIPC_SENDREC, 0, driver[which].endpt, &msg);

	if (r != 0) {
		/* Should we restart the driver now? */
		printk("Filter: driver_close: kipc_module_call returned %d\n", r);

		return RET_REDO;
	}

	if(msg.m_type != KCNR_TASK_REPLY || msg.REP_STATUS != 0) {
		printk("Filter: driver_close: kipc_module_call returned %d, %d\n",
			msg.m_type, msg.REP_STATUS);

		return RET_REDO;
	}

	return 0;
}

/*===========================================================================*
 *				driver_init				     *
 *===========================================================================*/
void driver_init(void)
{
	/* Initialize the driver layer. */
	int r;

	self_ep = getprocnr();

	memset(driver, 0, sizeof(driver));

	/* Endpoints unknown. */
	driver[DRIVER_MAIN].endpt = ENDPT_NONE;
	driver[DRIVER_BACKUP].endpt = ENDPT_NONE;

	/* Get disk driver's and this proc's endpoint. */
	driver[DRIVER_MAIN].label = MAIN_LABEL;
	driver[DRIVER_MAIN].minor = MAIN_MINOR;

	r = ds_retrieve_u32(driver[DRIVER_MAIN].label,
		(u32_t *) &driver[DRIVER_MAIN].endpt);
	if (r != 0) {
		printk("Filter: failed to get main disk driver's endpoint: "
			"%d\n", r);
		bad_driver(DRIVER_MAIN, BD_DEAD, -EFAULT);
		check_driver(DRIVER_MAIN);
	}
	else if (driver_open(DRIVER_MAIN) != 0) {
		panic(__FILE__, "unhandled driver_open failure", NO_NUM);
	}

	if(USE_MIRROR) {
		driver[DRIVER_BACKUP].label = BACKUP_LABEL;
		driver[DRIVER_BACKUP].minor = BACKUP_MINOR;

		if(!strcmp(driver[DRIVER_MAIN].label,
				driver[DRIVER_BACKUP].label)) {
			panic(__FILE__, "same driver: not tested", NO_NUM);
		}

		r = ds_retrieve_u32(driver[DRIVER_BACKUP].label,
			(u32_t *) &driver[DRIVER_BACKUP].endpt);
		if (r != 0) {
			printk("Filter: failed to get backup disk driver's "
				"endpoint: %d\n", r);
			bad_driver(DRIVER_BACKUP, BD_DEAD, -EFAULT);
			check_driver(DRIVER_BACKUP);
		}
		else if (driver_open(DRIVER_BACKUP) != 0) {
			panic(__FILE__, "unhandled driver_open failure",
				NO_NUM);
		}
	}
}

/*===========================================================================*
 *				driver_shutdown				     *
 *===========================================================================*/
void driver_shutdown(void)
{
	/* Clean up. */

#if DEBUG
	printk("Filter: %u driver deaths, %u protocol errors, "
		"%u data errors\n", problem_stats[BD_DEAD],
		problem_stats[BD_PROTO], problem_stats[BD_DATA]);
#endif

	if(driver_close(DRIVER_MAIN) != 0)
		printk("Filter: DEV_CLOSE failed on shutdown (1)\n");

	if(USE_MIRROR)
		if(driver_close(DRIVER_BACKUP) != 0)
			printk("Filter: DEV_CLOSE failed on shutdown (2)\n");
}

/*===========================================================================*
 *				get_raw_size				     *
 *===========================================================================*/
u64_t get_raw_size(void)
{
	/* Return the size of the raw disks as used by the filter driver.
	 */

	return disk_size;
}

/*===========================================================================*
 *				reset_kills				     *
 *===========================================================================*/
void reset_kills(void)
{
	/* Reset kill and retry statistics. */
	driver[DRIVER_MAIN].kills = 0;
	driver[DRIVER_MAIN].retries = 0;
	driver[DRIVER_BACKUP].kills = 0;
	driver[DRIVER_BACKUP].retries = 0;
}

/*===========================================================================*
 *				bad_driver				     *
 *===========================================================================*/
int bad_driver(int which, int type, int error)
{
	/* A disk driver has died or produced an error. Mark it so that we can
	 * deal with it later, and return RET_REDO to indicate that the
	 * current operation is to be retried. Also store an error code to
	 * return to the user if the situation is unrecoverable.
	 */
	driver[which].problem = type;
	driver[which].error = error;

	return RET_REDO;
}

/*===========================================================================*
 *				new_driver_ep				     *
 *===========================================================================*/
static int new_driver_ep(int which)
{
	/* See if a new driver instance has already been started for the given
	 * driver, by retrieving its entry from DS.
	 */
	int r;
	endpoint_t endpt;

	r = ds_retrieve_u32(driver[which].label, (u32_t *) &endpt);

	if (r != 0) {
		printk("Filter: DS query for %s failed\n",
			driver[which].label);

		return 0;
	}

	if (endpt == driver[which].endpt) {
#if DEBUG
		printk("Filter: same endpoint for %s\n", driver[which].label);
#endif
		return 0;
	}

#if DEBUG
	printk("Filter: new enpdoint for %s: %d -> %d\n", driver[which].label,
		driver[which].endpt, endpt);
#endif

	driver[which].endpt = endpt;

	return 1;
}

/*===========================================================================*
 *				check_problem				     *
 *===========================================================================*/
static int check_problem(int which, int problem, int retries, int *tell_rs)
{
	/* A problem has occurred with a driver. Update statistics, and decide
	 * what to do. If -EAGAIN is returned, the driver should be restarted;
	 * any other result will be passed up.
	 */

#if DEBUG
	printk("Filter: check_driver processing driver %d, problem %d\n",
		which, problem);
#endif

	problem_stats[problem]++;

	if(new_driver_ep(which)) {
#if DEBUG
		printk("Filter: check_problem: noticed a new driver\n");
#endif

		if(driver_open(which) == 0) {
#if DEBUG2
			printk("Filter: open 0 -> no recovery\n");
#endif
			return 0;
		} else {
#if DEBUG2
			printk("Filter: open not 0 -> recovery\n");
#endif
			problem = BD_PROTO;
			problem_stats[problem]++;
		}
	}

	/* If the driver has died, we always need to restart it. If it has
	 * been giving problems, we first retry the request, up to N times,
	 * after which we kill and restart the driver. We restart the driver
	 * up to M times, after which we remove the driver from the mirror
	 * configuration. If we are not set up to do mirroring, we can only
	 * do one thing, and that is continue to limp along with the bad
	 * driver..
	 */
	switch(problem) {
	case BD_PROTO:
	case BD_DATA:
		driver[which].retries++;

#if DEBUG
		printk("Filter: disk driver %d has had "
			"%d/%d retry attempts, %d/%d kills\n", which, 
			driver[which].retries, NR_RETRIES,
			driver[which].kills, NR_RESTARTS);
#endif

		if (driver[which].retries < NR_RETRIES) {
			if(retries == 1) {
#if DEBUG
				printk("Filter: not restarting; retrying "
					"(retries %d/%d, kills %d/%d)\n",
					driver[which].retries, NR_RETRIES,
					driver[which].kills, NR_RESTARTS);
#endif
				return 0;
			}
#if DEBUG
			printk("Filter: restarting (retries %d/%d, "
				"kills %d/%d, internal retry %d)\n",
				driver[which].retries, NR_RETRIES,
				driver[which].kills, NR_RESTARTS, retries);
#endif
		}

#if DEBUG
		printk("Filter: disk driver %d has reached error "
			"threshold, restarting driver\n", which);
#endif

		*tell_rs = 1;
		break;

	case BD_DEAD:
		/* Can't kill that which is already dead.. */
		*tell_rs = 0;
		break;

	default:
		panic(__FILE__, "invalid problem", problem);
	}

	/* At this point, the driver will be restarted. */
	driver[which].retries = 0;
	driver[which].kills++;

	if (driver[which].kills < NR_RESTARTS)
		return -EAGAIN;

	/* We've reached the maximum number of restarts for this driver. */
	if (USE_MIRROR) {
		printk("Filter: kill threshold reached, disabling mirroring\n");

		USE_MIRROR = 0;

		if (which == DRIVER_MAIN) {
			driver[DRIVER_MAIN] = driver[DRIVER_BACKUP];

			/* This is not necessary. */
			strcpy(MAIN_LABEL, BACKUP_LABEL);
			MAIN_MINOR = BACKUP_MINOR;
		}

		driver[DRIVER_BACKUP].endpt = ENDPT_NONE;

		return 0;
	}
	else {
		/* We tried, we really did. But now we give up. Tell the user.
		 */
		printk("Filter: kill threshold reached, returning error\n");

		if (driver[which].error == -EAGAIN) return -EIO;

		return driver[which].error;
	}
}

/*===========================================================================*
 *				restart_driver				     *
 *===========================================================================*/
static void restart_driver(int which, int tell_rs)
{
	/* Restart the given driver. Block until the new instance is up.
	 */
	kipc_msg_t msg;
	endpoint_t endpt;
	int r, w = 0;

	if (tell_rs) {
		/* Tell RS to refresh or restart the driver */
		msg.m_type = RS_REFRESH;
		msg.RS_CMD_ADDR = driver[which].label;
		msg.RS_CMD_LEN = strlen(driver[which].label);

#if DEBUG
		printk("Filter: asking RS to refresh %s..\n",
			driver[which].label);
#endif

		r = kipc_module_call(KIPC_SENDREC, 0, RS_PROC_NR, &msg);

		if (r != 0 || msg.m_type != 0)
			panic(__FILE__, "RS request failed", r);

#if DEBUG
		printk("Filter: RS call succeeded\n");
#endif
	}

	/* Wait until the new driver instance is up, and get its endpoint. */
#if DEBUG
	printk("Filter: endpoint update driver %d; old endpoint %d\n",
		which, driver[which].endpt);
#endif

	do {
		if(w) flt_sleep(1);
		w = 1;

		r = ds_retrieve_u32(driver[which].label, (u32_t *) &endpt);

#if DEBUG2
		if (r != 0)
			printk("Filter: DS request failed (%d)\n", r);
		else if (endpt == driver[which].endpt)
			printk("Filter: DS returned same endpoint\n");
		else
			printk("Filter: DS request 0, new endpoint\n");
#endif
	} while (r != 0 || endpt == driver[which].endpt);

	driver[which].endpt = endpt;
}

/*===========================================================================*
 *				check_driver				     *
 *===========================================================================*/
int check_driver(int which)
{
	/* See if the given driver has been troublesome, and if so, deal with
	 * it.
	 */
	int problem, tell_rs;
	int r, retries = 0;

	problem = driver[which].problem;

	if (problem == BD_NONE)
		return 0;

	do {
		if(retries) {
#if DEBUG
			printk("Filter: check_driver: retry number %d\n",
				retries);
#endif
			problem = BD_PROTO;
		}
		retries++;
		driver[which].problem = BD_NONE;

		/* Decide what to do: continue operation, restart the driver,
		 * or return an error.
		 */
		r = check_problem(which, problem, retries, &tell_rs);
		if (r != -EAGAIN)
			return r;

		/* Restarting the driver it is. First tell RS (if necessary),
		 * then wait for the new driver instance to come up.
		 */
		restart_driver(which, tell_rs);

		/* Finally, open the device on the new driver */
	} while (driver_open(which) != 0);

#if DEBUG
	printk("Filter: check_driver restarted driver %d, endpoint %d\n",
		which, driver[which].endpt);
#endif

	return 0;
}

/*===========================================================================*
 *				flt_senda				     *
 *===========================================================================*/
static int flt_senda(kipc_msg_t *mess, int which)
{
	/* Send a message to one driver. Can only return 0 at the moment. */
	int r;
	asynmsg_t *amp;

	/* Fill in the last bits of the message. */
	mess->DEVICE = driver[which].minor;
	mess->IO_ENDPT = self_ep;

	/* Send the message asynchronously. */
	amp = &amsgtable[which];
	amp->dst = driver[which].endpt;
	amp->msg = *mess;
	amp->flags = AMF_VALID;
	r = kipc_module_call(KIPC_SENDA, 2, 0, amsgtable);

	if(r != 0)
		panic(__FILE__, "send returned error", r);

	return r;
}

/*===========================================================================*
 *				check_senda				     *
 *===========================================================================*/
static int check_senda(int which)
{
	/* Check whether an earlier send resulted in an error indicating the
	 * message never got delivered. Only in that case can we reliably say
	 * that the driver died. Return BD_DEAD in this case, and BD_PROTO
	 * otherwise.
	 */
	asynmsg_t *amp;

	amp = &amsgtable[which];

	if ((amp->flags & AMF_DONE) &&
		(amp->result == -EDEADSRCDST || amp->result == -EDSTDIED)) {

		return BD_DEAD;
	}

	return BD_PROTO;
}

/*===========================================================================*
 *				flt_receive				     *
 *===========================================================================*/
static int flt_receive(kipc_msg_t *mess, int which)
{
	/* Receive a message from one or either driver, unless a timeout
	 * occurs. Can only return 0 or RET_REDO.
	 */
	int r;

	for (;;) {
		r = kipc_module_call(KIPC_RECEIVE, 0, ENDPT_ANY, mess);
		if(r != 0)
			panic(__FILE__, "kipc_module_call type KIPC_RECEIVE returned error", r);

		if(mess->m_source == CLOCK && is_notify(mess->m_type)) {
			if (mess->NOTIFY_TIMESTAMP < flt_alarm(-1)) {
#if DEBUG
				printk("Filter: SKIPPING old alarm "
					"notification\n");
#endif
				continue;
			}

#if DEBUG
			printk("Filter: timeout waiting for disk driver %d "
				"reply!\n", which);
#endif

			/* If we're waiting for either driver,
		 	 * both are at fault.
		 	 */
			if (which < 0) {
				bad_driver(DRIVER_MAIN,
					check_senda(DRIVER_MAIN), -EFAULT);

				return bad_driver(DRIVER_BACKUP,
					check_senda(DRIVER_BACKUP), -EFAULT);
			}

			/* Otherwise, just report the one not replying as dead.
			 */
			return bad_driver(which, check_senda(which), -EFAULT);
		}

		if (mess->m_source != driver[DRIVER_MAIN].endpt &&
				mess->m_source != driver[DRIVER_BACKUP].endpt) {
#if DEBUG
			printk("Filter: got STRAY message %d from %d\n",
				mess->m_type, mess->m_source);
#endif

			continue;
		}

		/* We are waiting for a reply from one specific driver. */
		if (which >= 0) {
			/* If the message source is that driver, good. */
			if (mess->m_source == driver[which].endpt)
				break;

			/* This should probably be treated as a real protocol
			 * error. We do not abort any receives (not even paired
			 * receives) except because of timeouts. Getting here
			 * means a driver replied at least the timeout period
			 * later than expected, which should be enough reason
			 * to kill it really. The other explanation is that it
			 * is actually violating the protocol and sending bogus
			 * messages...
			 */
#if DEBUG
			printk("Filter: got UNEXPECTED reply from %d\n",
				mess->m_source);
#endif

			continue;
		}

		/* We got a message from one of the drivers, and we didn't
		 * care which one we wanted to receive from. A-0.
		 */
		break;
	}

	return 0;
}

/*===========================================================================*
 *				flt_sendrec				     *
 *===========================================================================*/
static int flt_sendrec(kipc_msg_t *mess, int which)
{
	int r;

	r = flt_senda(mess, which);
	if(r != 0)
		return r;

	if(check_senda(which) == BD_DEAD) {
		return bad_driver(which, BD_DEAD, -EFAULT);
	}

	/* Set alarm. */
	flt_alarm(DRIVER_TIMEOUT);

	r = flt_receive(mess, which);

	/* Clear the alarm. */
	flt_alarm(0);
	return r;
}

/*===========================================================================*
 *				do_sendrec_both				     *
 *===========================================================================*/
static int do_sendrec_both(kipc_msg_t *m1, kipc_msg_t *m2)
{
	/* If USEE_MIRROR is set, call flt_sendrec() to both drivers.
	 * Otherwise, only call flt_sendrec() to the main driver.
	 * This function will only return either 0 or RET_REDO.
	 */
	int r, which = -1;
	kipc_msg_t ma, mb;

	/* If the two disks use the same driver, call flt_sendrec() twice
	 * sequentially. Such a setup is not very useful though.
	 */
	if (!strcmp(driver[DRIVER_MAIN].label, driver[DRIVER_BACKUP].label)) {
		if ((r = flt_sendrec(m1, DRIVER_MAIN)) != 0) return r;
		return flt_sendrec(m2, DRIVER_BACKUP);
	}

	/* If the two disks use different drivers, call flt_senda()
	 * twice, and then flt_receive(), and distinguish the return
	 * messages by means of m_source.
	 */
	if ((r = flt_senda(m1, DRIVER_MAIN)) != 0) return r;
	if ((r = flt_senda(m2, DRIVER_BACKUP)) != 0) return r;

	/* Set alarm. */
	flt_alarm(DRIVER_TIMEOUT);

	/* The message received by the 1st flt_receive() may not be
	 * from DRIVER_MAIN.
	 */
	if ((r = flt_receive(&ma, -1)) != 0) {
		flt_alarm(0);
		return r;
	}

	if (ma.m_source == driver[DRIVER_MAIN].endpt) {
		which = DRIVER_BACKUP;
	} else if (ma.m_source == driver[DRIVER_BACKUP].endpt) {
		which = DRIVER_MAIN;
	} else {
		panic(__FILE__, "message from unexpected source",
			ma.m_source);
	}

	r = flt_receive(&mb, which);

	/* Clear the alarm. */
	flt_alarm(0);

	if(r != 0)
		return r;

	if (ma.m_source == driver[DRIVER_MAIN].endpt) {
		*m1 = ma;
		*m2 = mb;
	} else {
		*m1 = mb;
		*m2 = ma;
	}

	return 0;
}

/*===========================================================================*
 *				do_sendrec_one				     *
 *===========================================================================*/
static int do_sendrec_one(kipc_msg_t *m1, kipc_msg_t *m2)
{
	/* Only talk to the main driver. If something goes wrong, it will
	 * be fixed elsewhere.
	 * This function will only return either 0 or RET_REDO.
	 */

    	return flt_sendrec(m1, DRIVER_MAIN);
}

/*===========================================================================*
 *				paired_sendrec				     *
 *===========================================================================*/
static int paired_sendrec(kipc_msg_t *m1, kipc_msg_t *m2, int both)
{
	/* Sendrec with the disk driver. If the disk driver is down, and was
	 * restarted, redo the request, until the driver works fine, or can't
	 * be restarted again.
	 */
	int r;

#if DEBUG2
	printk("paired_sendrec(%d) - <%d,%x:%x,%d> - %x,%x\n",
		both, m1->m_type, m1->HIGHPOS, m1->POSITION, m1->COUNT,
		m1->IO_GRANT, m2->IO_GRANT);
#endif

	if (both)
		r = do_sendrec_both(m1, m2);
	else
		r = do_sendrec_one(m1, m2);

#if DEBUG2
	if (r != 0)
		printk("paired_sendrec about to return %d\n", r);
#endif

	return r;
}

/*===========================================================================*
 *				single_grant				     *
 *===========================================================================*/
static int single_grant(endpoint_t endpt, vir_bytes buf, int access,
	cp_grant_id_t *gid, iovec_s_t vector[NR_IOREQS], size_t *sizep)
{
	/* Create grants for a vectored request to a single driver.
	 */
	cp_grant_id_t grant;
	size_t size, chunk;
	int count;

	size = *sizep;

	/* Split up the request into chunks, if requested. This makes no
	 * difference at all, except that this works around a weird performance
	 * bug with large DMA PRDs on some machines.
	 */
	if (CHUNK_SIZE > 0) chunk = CHUNK_SIZE;
	else chunk = size;

	/* Fill in the vector, creating a grant for each item. */
	for (count = 0; size > 0 && count < NR_IOREQS; count++) {
		/* The last chunk will contain all the remaining data. */
		if (chunk > size || count == NR_IOREQS - 1)
			chunk = size;

		grant = cpf_grant_direct(endpt, buf, chunk, access);
		if (!GRANT_VALID(grant))
			panic(__FILE__, "invalid grant", grant);

		vector[count].iov_grant = grant;
		vector[count].iov_size = chunk;

		buf += chunk;
		size -= chunk;
	}

	/* Then create a grant for the vector itself. */
	*gid = cpf_grant_direct(endpt, (vir_bytes) vector,
		sizeof(vector[0]) * count, CPF_READ | CPF_WRITE);

	if (!GRANT_VALID(*gid))
		panic(__FILE__, "invalid grant", *gid);

	return count;
}

/*===========================================================================*
 *				paired_grant				     *
 *===========================================================================*/
static int paired_grant(char *buf1, char *buf2, int request,
	cp_grant_id_t *gids, iovec_s_t vectors[2][NR_IOREQS], size_t *sizes,
	int both)
{
	/* Create memory grants, either to one or to both drivers.
	 */
	cp_grant_id_t gid;
	int count, access;

	count = 0;
	access = (request == FLT_WRITE) ? CPF_READ : CPF_WRITE;

	if(driver[DRIVER_MAIN].endpt > 0) {
		count = single_grant(driver[DRIVER_MAIN].endpt,
			(vir_bytes) buf1, access, &gids[0], vectors[0],
			&sizes[0]);
	}

	if (both) {
		if(driver[DRIVER_BACKUP].endpt > 0) {
			count = single_grant(driver[DRIVER_BACKUP].endpt,
				(vir_bytes) buf2, access, &gids[1],
				vectors[1], &sizes[1]);
		}
		}
	}

/*===========================================================================*
 *				single_revoke				     *
 *===========================================================================*/
void single_revoke(cp_grant_id_t gid, iovec_s_t vector[NR_IOREQS],
	size_t *sizep, int count)
{
	/* Revoke all grants associated with a request to a single driver.
	 * Modify the given size to reflect the actual I/O performed.
	 */
	int i;

	/* Revoke the grants for all the elements of the vector. */
	for (i = 0; i < count; i++) {
		cpf_revoke(vector[i].iov_grant);
		*sizep -= vector[i].iov_size;
	}

	/* Then revoke the grant for the vector itself. */
	cpf_revoke(gid);
}

/*===========================================================================*
 *				paired_revoke				     *
 *===========================================================================*/
static void paired_revoke(cp_grant_id_t *gids, iovec_s_t vectors[2][NR_IOREQS],
	size_t *sizes, int count, int both)
{
	/* Revoke grants to drivers for a single request.
	 */

	single_revoke(gids[0], vectors[0], &sizes[0], count);

	if (both)
		single_revoke(gids[1], vectors[1], &sizes[1], count);
}

/*===========================================================================*
 *				read_write				     *
 *===========================================================================*/
int read_write(u64_t pos, char *bufa, char *bufb, size_t *sizep, int request)
{
	iovec_s_t vectors[2][NR_IOREQS];
	kipc_msg_t m1, m2;
	cp_grant_id_t gids[2];
	size_t sizes[2];
	int r, both, count;

	gids[0] = gids[1] = GRANT_INVALID;
	sizes[0] = sizes[1] = *sizep;

	/* Send two requests only if mirroring is enabled and the given request
	 * is either FLT_READ2 or FLT_WRITE.
	 */
	both = (USE_MIRROR && request != FLT_READ);

	count = paired_grant(bufa, bufb, request, gids, vectors, sizes, both);

	m1.m_type = (request == FLT_WRITE) ? DEV_SCATTER_S : DEV_GATHER_S;
	m1.COUNT = count;
	m1.POSITION = ex64lo(pos);
	m1.HIGHPOS = ex64hi(pos);

	m2 = m1;

	m1.IO_GRANT = (char *) gids[0];
	m2.IO_GRANT = (char *) gids[1];

	r = paired_sendrec(&m1, &m2, both);

	paired_revoke(gids, vectors, sizes, count, both);

	if(r != 0) {
#if DEBUG
		if (r != RET_REDO)
			printk("Filter: paired_sendrec returned %d\n", r);
#endif
		return r;
	}

	if (m1.m_type != KCNR_TASK_REPLY || m1.REP_STATUS != 0) {
		printk("Filter: unexpected/invalid reply from main driver: "
			"(%x, %d)\n", m1.m_type, m1.REP_STATUS);

		return bad_driver(DRIVER_MAIN, BD_PROTO,
			(m1.m_type == KCNR_TASK_REPLY) ? m1.REP_STATUS : -EFAULT);
	}

	if (sizes[0] != *sizep) {
		printk("Filter: truncated reply from main driver\n");

		/* If the driver returned a value *larger* than we requested,
		 * OR if we did NOT exceed the disk size, then we should
		 * report the driver for acting strangely!
		 */
		if (sizes[0] < 0 || sizes[0] > *sizep ||
			cmp64(add64u(pos, sizes[0]), disk_size) < 0)
			return bad_driver(DRIVER_MAIN, BD_PROTO, -EFAULT);

		/* Return the actual size. */
		*sizep = sizes[0];
	}

	if (both) {
		if (m2.m_type != KCNR_TASK_REPLY || m2.REP_STATUS != 0) {
			printk("Filter: unexpected/invalid reply from "
				"backup driver (%x, %d)\n",
				m2.m_type, m2.REP_STATUS);

			return bad_driver(DRIVER_BACKUP, BD_PROTO,
				m2.m_type == KCNR_TASK_REPLY ? m2.REP_STATUS :
				-EFAULT);
		}
		if (sizes[1] != *sizep) {
			printk("Filter: truncated reply from backup driver\n");

			/* As above */
			if (sizes[1] < 0 || sizes[1] > *sizep ||
				cmp64(add64u(pos, sizes[1]), disk_size) < 0)
				return bad_driver(DRIVER_BACKUP, BD_PROTO,
					-EFAULT);

			/* Return the actual size. */
			if (*sizep >= sizes[1])
				*sizep = sizes[1];
		}
	}

	return 0;
}
