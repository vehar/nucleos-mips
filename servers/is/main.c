/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* System Information Service. 
 * This service handles the various debugging dumps, such as the process
 * table, so that these no longer directly touch kernel memory. Instead, the 
 * system task is asked to copy some table in local memory. 
 * 
 * Created:
 *   Apr 29, 2004	by Jorrit N. Herder
 */

#include "inc.h"
#include <nucleos/endpoint.h>

/* Allocate space for the global variables. */
kipc_msg_t m_in;		/* the input message itself */
kipc_msg_t m_out;		/* the output message used for reply */
int who_e;		/* caller's proc number */
int callnr;		/* system call number */

extern int errno;	/* error number set by system library */

/* Declare some local functions. */
static void init_server(int argc, char **argv);
static void sig_handler(void);
static void get_work(void);
static void reply(int whom, int result);

/*===========================================================================*
 *				main                                         *
 *===========================================================================*/
int main(int argc, char **argv)
{
/* This is the main routine of this service. The main loop consists of 
 * three major activities: getting new work, processing the work, and
 * sending the reply. The loop never terminates, unless a panic occurs.
 */
  int result;                 
  sigset_t sigset;

  /* Initialize the server, then go to work. */
  init_server(argc, argv);

  /* Main loop - get work and do it, forever. */         
  while (TRUE) {              

      /* Wait for incoming message, sets 'callnr' and 'who'. */
      get_work();

      if (is_notify(callnr)) {
	      switch (_ENDPOINT_P(who_e)) {
		      case SYSTEM:
			      printk("got message from SYSTEM\n");
			      sigset = m_in.NOTIFY_ARG;
			      for ( result=0; result< _NSIG; result++) {
				      if (sigismember(&sigset, result))
					      printk("signal %d found\n", result);
			      }
			      continue;
		      case PM_PROC_NR:
		              sig_handler();
			      continue;
		      case TTY_PROC_NR:
			      result = do_fkey_pressed(&m_in);
			      break;
		      case RS_PROC_NR:
			      kipc_module_call(KIPC_NOTIFY, 0, m_in.m_source, 0);
			      continue;
	      }
      }
      else {
          printk("IS: warning, got illegal request %d from %d\n",
          	callnr, m_in.m_source);
          result = -EDONTREPLY;
      }

      /* Finally send reply message, unless disabled. */
      if (result != -EDONTREPLY) {
	  reply(who_e, result);
      }
  }
  return 0;				/* shouldn't come here */
}

/*===========================================================================*
 *				 init_server                                 *
 *===========================================================================*/
static void init_server(int argc, char **argv)
{
/* Initialize the information service. */
  struct sigaction sigact;

  /* Install signal handler. Ask PM to transform signal into message. */
  sigact.sa_handler = SIG_MESS;
  sigact.sa_mask = ~0;			/* block all other signals */
  sigact.sa_flags = 0;			/* default behaviour */
  if (sigaction(SIGTERM, &sigact, NULL) < 0) 
      report("IS","warning, sigaction() failed", errno);

  /* Set key mappings. */
  map_unmap_fkeys(TRUE /*map*/);
}

/*===========================================================================*
 *				sig_handler                                  *
 *===========================================================================*/
static void sig_handler()
{
  sigset_t sigset;

  /* Try to obtain signal set from PM. */
  if (getsigset(&sigset) != 0) return;

  /* Only check for termination signal. */
  if (!sigismember(&sigset, SIGTERM)) return;

  /* Shutting down. Unset key mappings, and quit. */
  map_unmap_fkeys(FALSE /*map*/);

  exit(0);
}

/*===========================================================================*
 *				get_work                                     *
 *===========================================================================*/
static void get_work()
{
    int status = 0;
    status = kipc_module_call(KIPC_RECEIVE, 0, ENDPT_ANY, &m_in);   /* this blocks until message arrives */
    if (status != 0)
        panic("IS","failed to receive message!", status);
    who_e = m_in.m_source;        /* message arrived! set sender */
    callnr = m_in.m_type;       /* set function call number */
}

/*===========================================================================*
 *				reply					     *
 *===========================================================================*/
static void reply(who, result)
int who;                           	/* destination */
int result;                           	/* report result to replyee */
{
    int send_status;
    m_out.m_type = result;  		/* build reply message */
    send_status = kipc_module_call(KIPC_SEND, 0, who, &m_out);    /* send the message */
    if (send_status != 0)
        panic("IS", "unable to send reply!", send_status);
}

