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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "smsalive.h"

#define NUM_PARAMS 13
static char *config_params[NUM_PARAMS] = { "logfile", "verbosity", "daemonize","pid_file",
	"sms_activated","sipgate_user","sipgate_pass","cellphone",
	"host1","host2","host3","host4","failcount"};


int loadConfig(char *conf)
{
	FILE *config_file;
	char line[120];
	char value[100];
	char *lpos;
	int param;

	config_file = fopen(conf,"r");
	if(!config_file)
	{
		return 0;
	}

	/* set everything to zero */
	memset(&config, 0, sizeof(config));
	

	/* step through every line */
	while(fgets(line, sizeof(line), config_file) != NULL)
	{
		/* skip comments and empty lines */
		if(line[0] == '#' || line[0] == '\n')
			continue;
		for(param = 0; param < NUM_PARAMS; param++)
		{
			/* if param name not at the beginning of line */
			if(strstr(line,config_params[param]) != line)
				continue;
			/* go beyond the = */
			if(!(lpos =  strstr(line, "=")))
				continue;
			/* go to the beginning of value 
			 * only whitespaces are skipped, no tabs */
			do
				lpos++;
			while(*lpos == ' ');
			
			strcpy(value, lpos);

			/* throw away carriage return 
			 * might only work for *nix */
			lpos = strchr(value,'\n');
			*lpos = 0;

			/* put the value where it belongs */
			switch(param)
			{
				/* logfile */
				case 0: strcpy(config.logfile, value);
					 break;
				/* verbosity */
				case 1: config.verbosity = atoi(value);
					 break;
				/* daemonize? */
				case 2: config.daemonize = atoi(value);
					 break;
				/* pid file */
				case 3: strcpy(config.pid_file, value);
					 break;
				/* sms activated */
				case 4: config.sms_activated = atoi(value);
					break;
				/* sipgate user */
				case 5: strcpy(config.sipgate_user, value);
					 break;
				/* sipgate pass */
				case 6: strcpy(config.sipgate_pass, value);
					 break;
				/* cellphone number */
				case 7: strcpy(config.cellphone, value);
					 break;
				/* host1 */
				case 8:	strcpy(config.host[0], value);
					break;
				/* host2 */
				case 9:	strcpy(config.host[1], value);
					break;
				/* host3 */
				case 10:strcpy(config.host[2], value);
					break;
				/* host4 */
				case 11:strcpy(config.host[3], value);
					break;
				/* failcount */
				case 12: config.failcount = atoi(value);
					break;
			}
		}
	}

	fclose(config_file);
	return 1;
}

