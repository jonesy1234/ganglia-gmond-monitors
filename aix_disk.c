#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <libperfstat.h>
#include <sys/systemcfg.h>

/* See /usr/include/sys/iplcb.h to explain the below */
#define XINTFRAC        ((double)(_system_configuration.Xint)/(double)(_system_configuration.Xfrac))

/* hardware ticks per millisecond */
#define HWTICS2MSECS(x)    ((double)x * XINTFRAC)/1000000.0

#define MAX_BUF_SIZE 1024

#ifndef FIRST_DISK
#define FIRST_DISK ""
#endif

call_gmetric_float(metric_name, metric_name_append, metric_value)
char    *metric_name;
char    *metric_name_append;
float   metric_value;
{
        char  buf[MAX_BUF_SIZE];
        char gmetric_command[1025];
        int status;
        FILE *f;

        sprintf(gmetric_command, "/opt/freeware/bin/gmetric --name=%s_%s --value=%0.1f --type=int32 --dmax=60", metric_name, metric_name_append, metric_value);
        /* printf("The following stats will be posted - %-16s\n",gmetric_command); */

        f = popen( gmetric_command, "r");
        status = pclose(f);

        if (status > 0) {
                /* Error reported by gmetric */
                printf("gmetric reported and error\n");
        }
}

char *fix(char *s)
{
        int j;
        for(j=0;j<IDENTIFIER_LENGTH;j++) {
                if( !(isalpha(s[j]) ||
                        isdigit(s[j]) ||
                        s[j] == '-' ||
                        s[j] == '_' ||
                        s[j] == ' '
                     ) ) {
                        s[j] = 0;
                        break;
                }
        }
        return s;
}

int main(int argc, char* argv[])
{
        int i, j, ret, disks;
        perfstat_disk_t *a;
        perfstat_disk_t *b;
        perfstat_id_t name;
        char *substring;
        int elapsed=1;
        int ncpus;
        pid_t pid, ppid;
        gid_t gid;
        FILE *pid_file;

        /* get the process id */
        if ((pid = getpid()) < 0) {
                perror("unable to get pid");
        } else {
                pid_file=fopen("/var/run/gmond_iostats.pid", "w+");
                fprintf(pid_file, "%d", pid);
                fclose(pid_file);
        }


        /* get the number of CPUs */
        ncpus=perfstat_cpu(NULL, NULL, sizeof(perfstat_cpu_t), 0);

        /* check how many perfstat_disk_t structures are available */
        disks = perfstat_disk(NULL, NULL, sizeof(perfstat_disk_t), 0);
        if(disks==-1) {
                perror("perfstat_disk(NULL)");
                exit(3);
        }

        /* allocate enough memory for all the structures */
        a = malloc( sizeof(perfstat_disk_t) * disks);
        b = malloc( sizeof(perfstat_disk_t) * disks);

        /* ask to get all the structures available in one call */
        /* return code is number of structures returned */
        strcpy(name.name,FIRST_DISK);
        ret = perfstat_disk(&name, a, sizeof(perfstat_disk_t), disks);

        if(disks==-1) {
                perror("perfstat_disk(a)");
                exit(4);
        }

        for(;;) {
#define DELTA(member) (b[i].member - a[i].member)
                sleep(elapsed);
                strcpy(name.name,FIRST_DISK);
                ret = perfstat_disk(&name, b, sizeof(perfstat_disk_t), disks);

                if(ret==-1) {
                        perror("perfstat_disk(b)");
                        exit(4);
                }

                for (i = 0; i < ret; i++) {
#define NONZERO(x) ((x)?(x):1)
                        call_gmetric_float(fix(b[i].name), "avgtime", (double)(HWTICS2MSECS(DELTA(wq_time))/NONZERO(DELTA(xfers))/elapsed));
                        call_gmetric_float(fix(b[i].name), "avgWQsz", (double)(DELTA(wq_sampled))/(100.0*(double)elapsed*(double)ncpus));
                        call_gmetric_float(fix(b[i].name), "avgSQsz", (double)(DELTA(q_sampled))/(100.0*(double)elapsed*(double)ncpus));
                        call_gmetric_float(fix(b[i].name), "SQfull",  DELTA(q_full));

                        /* Enabled for debugging
                        printf("%-16s               avgtime avgWQsz avgSQsz SQfull\n",fix(b[i].name));
                        printf("                          %8.1f %7.1f %7.1f %3llu \n",
                                (double)(HWTICS2MSECS(DELTA(wq_time))/NONZERO(DELTA(xfers))/elapsed),
                                (double)(DELTA(wq_sampled))/(100.0*(double)elapsed*(double)ncpus),
                                (double)(DELTA(q_sampled))/(100.0*(double)elapsed*(double)ncpus),
                                DELTA(q_full)); */
                }
                memcpy(a,b,sizeof(perfstat_disk_t) * disks );
        }
}
