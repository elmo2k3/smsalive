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
#include <unistd.h>
#include <sys/signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sysexits.h>

#include "smsalive.h"
#include "config.h"
#include "sms.h"

static char *monthToName[12] = {"Jan","Feb","Mar","Apr","May",
	"Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

static int killDaemon(int signal);
static int fileExists(const char *filename);
static void printUsage(void);
static void hadSignalHandler(int signal);

static int fileExists(const char *filename)
{
	FILE *fp = fopen(filename,"r");
	if(fp)
	{
		fclose(fp);
		return 1;
	}
	else
		return 0;
}

char *theTime(void)
{
	static char returnValue[9];
	time_t currentTime;
	struct tm *ptm;

	time(&currentTime);

	ptm = localtime(&currentTime);

	sprintf(returnValue,"%s %2d %02d:%02d:%02d",monthToName[ptm->tm_mon], ptm->tm_mday, 
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	return returnValue;
}

static void printUsage(void)
{
	printf("Usage :\n\n");
	printf("had --help  this text\n");
	printf("had -s      start (default)\n");
	printf("had -k      kill the daemon\n");
	printf("had -r      reload config (does currently not reconnect db and mpd\n\n");
}

static int killDaemon(int signal)
{
	FILE *pid_file = fopen(config.pid_file,"r");

	int pid;

	if(!pid_file)
	{
		printf("Could not open %s. Maybe had is not running?\n",config.pid_file);
		return(EXIT_FAILURE);
	}
	fscanf(pid_file,"%d",&pid);
	fclose(pid_file);

	kill(pid,signal);
	return EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
	signal(SIGINT, (void*)hadSignalHandler);
	signal(SIGTERM, (void*)hadSignalHandler);

	int host_reachable[4];
	int host_failcount[4];

	memset(&host_reachable, 1, sizeof(host_reachable));
	memset(&host_failcount, 1, sizeof(host_failcount));

	pid_t pid;
	FILE *pid_file;
	
	if(!loadConfig(SMSALIVE_CONFIG_FILE))
	{
		verbose_printf(0,"Could not load config ... aborting\n\n");
		exit(EX_NOINPUT);
	}



	if(argc > 2)
	{
		printUsage();
		exit(EXIT_FAILURE);
	}
	
	if(argc >1)
	{
		if(!strcmp(argv[1],"--help"))
		{
			printUsage();
			exit(EXIT_SUCCESS);
		}

		if(!strcmp(argv[1],"-k"))
		{
			exit(killDaemon(SIGTERM));
		}

		/* reload config */
		if(!strcmp(argv[1],"-r"))
			exit(killDaemon(SIGHUP));
	}

	if(config.daemonize)
	{
		if(fileExists(config.pid_file))
		{
			printf("%s exists. Maybe had is still running?\n",config.pid_file);
			exit(EXIT_FAILURE);
		}

		if(( pid = fork() ) != 0 )
			exit(EX_OSERR);

		if(setsid() < 0)
			exit(EX_OSERR);
		
		signal(SIGHUP, SIG_IGN);
		
		if(( pid = fork() ) != 0 )
			exit(EX_OSERR);

		umask(0);
		
		signal(SIGHUP, (void*)hadSignalHandler);

		pid_file = fopen(config.pid_file,"w");
		if(!pid_file)
		{
			printf("Could not write %s\n",config.pid_file);
			exit(EXIT_FAILURE);
		}
		fprintf(pid_file,"%d\n",(int)getpid());
		fclose(pid_file);

		if(config.verbosity >= 9)
			printf("My PID is %d\n",(int)getpid());


		
		freopen(config.logfile, "a", stdout);
		freopen(config.logfile, "a", stderr);

		/* write into file without buffer */
		setvbuf(stdout, NULL, _IONBF, 0);
		setvbuf(stderr, NULL, _IONBF, 0);

	}
	

	verbose_printf(0, "smsalive gestartet\n");

	int status;
	int i;

	while(1)
	{
		char buf[1024];

		i=0;
		while(config.host[i][0])
		{
			sprintf(buf,"ping -c1 %s > /dev/null 2>&1",config.host[i]);
			status = system(buf);
			if(status)
				host_failcount[i]++;
			else
				host_failcount[i] = 0;

			if(host_failcount[i])
				verbose_printf(0,"failcount for %s is %d\n",config.host[i], host_failcount[i]);
			if(host_failcount[i] >= config.failcount && host_reachable[i])
			{
				host_reachable[i] = 0;
				verbose_printf(0,"host %s not reachable!\n",config.host[i]);
				sprintf(buf,"%s  host %s is not reachable!\n",theTime(), config.host[i]);
				sms(buf);
			}
			else if(!host_failcount[i] && !host_reachable[i])
			{
				host_reachable[i] = 1;
				sprintf(buf,"%s  host %s is back!\n",theTime(), config.host[i]);
				sms(buf);
			}
			i++;
		}
		sleep(60);
	}

	return 0;
}

static void hadSignalHandler(int signal)
{
	if(signal == SIGTERM || signal == SIGINT)
	{
		if(config.daemonize)
			unlink(config.pid_file);
		verbose_printf(0,"Shutting down\n");
		exit(EXIT_SUCCESS);
	}
	else if(signal == SIGHUP)
	{
		struct _config configTemp;
		memcpy(&configTemp, &config, sizeof(config));

		verbose_printf(0,"Config reloaded\n");
		loadConfig(SMSALIVE_CONFIG_FILE);

	}
}
