/*
 * Copyright (C) 2007-2008 Bjoern Biesenbach <bjoern@bjoern-b.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __HAD_H__
#define __HAD_H__

#include <pthread.h>
#include <inttypes.h>

/** @file had.h
 */

#define SMSALIVE_CONFIG_FILE "/etc/smsalive.conf"


#define verbose_printf(X,args...) \
	if(X <= config.verbosity) \
        {\
                printf("%s    ",theTime()); \
                printf(args); \
	}


extern char *theTime(void);

/** Struct holding all config vars
 * 
 */
struct _config
{
	char pid_file[100]; /**< had pid file */
	char logfile[100]; /**< had logfile */
	int verbosity; /**< verbosity. currently 0 and 9 supported */
	int daemonize; /**< detach from tty, 0 or 1 */

	int sms_activated;
	char sipgate_user[100];
	char sipgate_pass[100];
	char cellphone[100];

	char host[5][100];
	int failcount;

}config;

#endif
