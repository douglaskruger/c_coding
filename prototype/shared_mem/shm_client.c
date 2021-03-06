/* =============================================================================
   Copyright © 2019 Skynet Consulting Ltd.

   This file is the property of Skynet Consulting Ltd. and shall not be reproduced,
   copied, or used as the basis for the manufacture or sale of equipment without
   the express written permission of Skynet Consulting Ltd.
   =============================================================================
*/
static const char module_id[] __attribute__((used)) = "$Id: shm_client.c$";
/*
 * Description:
 * Basic shared memory program - that uses a fork. One forked process will write and
 * the other will read.
 */
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <errno.h>
#include "general_socket.h"

#define WRITER_ID 33

#define TIME_H struct timespec
 
struct sockaddr_in    addr_con;
struct in_addr        localInterface;
struct ip_mreq        group;

int
create_shared_mem(int size)
{
    // ftok to generate unique key 
    key_t key = ftok("shmfile",65); 
  
    // shmget returns an identifier in shmid
    int shmid = shmget(key, size, 0666|IPC_CREAT);
    if (shmid < 0)
    {
        printf("Could not get shared memory. Error=%d\n. Exiting.\n\n", errno);
        exit(1);
    }
    return shmid;
}

void
detach_and_destroy_shared_mem(void * mem_ptr, int shmid)
{
    //detach from shared memory  
    shmdt(mem_ptr); 
    
    // destroy the shared memory 
    shmctl(shmid, IPC_RMID, NULL); 
}     

int
create_server_socket()
{
    int                   sockfd;
                       
    // socket()
    sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL);

    if (sockfd < 0)
    {
        perror("Error opening datagram socket");
        exit(1);
    }

    /* Initialize the group sockaddr structure with a group address */
    memset((char *) &addr_con, 0, sizeof(addr_con));
    addr_con.sin_family = AF_INET;
    addr_con.sin_port = htons(COMM_PORT);

    /* Disable loopback so you do not receive your own datagrams.  */
    addr_con.sin_addr.s_addr = inet_addr(MULTICAST_GROUP_IP);
    char loopch=0;

    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch,
        sizeof(loopch)) < 0)
    {
        perror("setting IP_MULTICAST_LOOP:");
        close(sockfd);
        exit(1);
    }
    localInterface.s_addr = inet_addr(CLIENT_IP);
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface,
        sizeof(localInterface)) < 0)
    {
        perror("setting local interface");
        exit(1);
    }
    return sockfd;
}

int
create_client_socket()
{
    struct ip_mreq        group;
    int                   sockfd;

    // socket()
    sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL);

    if (sockfd < 0)
    {
        perror("Error opening datagram socket");
        exit(1);
    }

    /* Initialize the group sockaddr structure with a group address */
    memset((char *) &addr_con, 0, sizeof(addr_con));
    addr_con.sin_family = AF_INET;
    addr_con.sin_port = htons(COMM_PORT);
    addr_con.sin_addr.s_addr = INADDR_ANY;

    /*
     * Enable SO_REUSEADDR to allow multiple instances of this
     * application to receive copies of the multicast datagrams.
     */
    int reuse=1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse,
        sizeof(reuse)) < 0)
    {
        perror("setting SO_REUSEADDR");
        close(sockfd);
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr*)&addr_con, sizeof(addr_con)))
    {
        perror("binding datagram socket");
        close(sockfd);
        exit(1);
    }
    /*
     * Join the multicast group 225.1.1.1 on the local 9.5.1.1
     * interface.  Note that this IP_ADD_MEMBERSHIP option must be
     * called for each local interface over which the multicast
     * datagrams are to be received.
     */
    group.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP_IP);
    group.imr_interface.s_addr = inet_addr(CLIENT_IP);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group,
        sizeof(group)) < 0)
    {
        perror("adding multicast group");
        close(sockfd);
        exit(1);
    }
    return sockfd;
}

int
close_socket(int sockfd)
{
    close(sockfd);
    return TRUE;
}

int
write_socket(int sockfd, void * data_ptr, int data_size)
{
    if (sendto(sockfd, data_ptr, data_size, 0,
         (struct sockaddr*)&addr_con, sizeof(addr_con)) < 0)
    {
        perror("Error sending datagram");
    }
    return TRUE;
}

int
read_socket(int sockfd, void * data_ptr, int data_size)
{
    if (read(sockfd, data_ptr, data_size) < 0)
    {
        perror("reading datagram message");
        close(sockfd);
        exit(1);
    }
    return TRUE;
}


typedef struct TEM_IO
{
    int    ObjectIndex;
    char   Name[100];
    TIME_H Name_t;
    int    Name_w;
    double Voltage;
    TIME_H Voltage_t;
    int    Voltage_w;
    double Current;
    TIME_H Current_t;
    int    Current_w;
    double Temperature;
    TIME_H Temperature_t;
    int    Temperature_w;
    double StateOfCharge;
    TIME_H StateOfCharge_t;
    int    StateOfCharge_w;
} tzTEM_IO;

void 
write_object(tzTEM_IO * tem, int i)
{
    struct timespec now; 
    clock_gettime(CLOCK_REALTIME, &now); 

    tem[i].ObjectIndex = i;
    sprintf(tem[i].Name, "TEM I/O-%d", i);
    tem[i].Name_t = now;
    tem[i].Name_w = WRITER_ID;

    tem[i].Voltage = rand() % 20;
    tem[i].Voltage_t = now;
    tem[i].Voltage_w = WRITER_ID;

    tem[i].Current = rand() % 20;
    tem[i].Current_t = now;
    tem[i].Current_w = WRITER_ID;

    tem[i].Temperature = rand() % 20;
    tem[i].Temperature_t = now;
    tem[i].Temperature_w = WRITER_ID;

    tem[i].StateOfCharge = rand() % 20;
    tem[i].StateOfCharge_t = now;
    tem[i].StateOfCharge_w = WRITER_ID;
}

void
read_object(tzTEM_IO * tem, int i)
{
        struct tm       curTm;
        char            timestr[100];

        gmtime_r(&(tem[i].Name_t.tv_sec), &curTm);
        sprintf(timestr, "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
               curTm.tm_year + 1900, curTm.tm_mon + 1, curTm.tm_mday,
               curTm.tm_hour, curTm.tm_min, curTm.tm_sec, tem[i].Name_t.tv_nsec / 1000000);

        printf("Object Index=%d\n",i);
        printf("  Name: %s at %s by %d\n", tem[i].Name, timestr, tem[i].Name_w);
        printf("  Voltage: %f\n", tem[i].Voltage);
        printf("  Current: %f\n", tem[i].Current);
        printf("  Temperature: %f\n", tem[i].Temperature);
        printf("  StateOfCharge: %f\n", tem[i].StateOfCharge);
}

int main() 
{ 
    time_t 		  t;

    /* Intializes random number generator */
    srand((unsigned) time(&t));

#ifdef SERVER
    int sockfd = create_server_socket();
#else
    int sockfd = create_client_socket();
#endif
    int shmid = create_shared_mem(4096000);

    // shmat to attach to shared memory 
    tzTEM_IO *tem = (tzTEM_IO *) shmat(shmid,(void*)0,0);
  
    /* Populate the initial values of local memory */
    printf("*** Initialization\n");
    for (int i=0; i<10000; i++)
    {
	write_object(tem, i);
	if (i % 100000 == 0) 
	    read_object(tem, i);
    }

    int i=0;
    int index;
    while (1) 
    {
	/* count up to 10,000 and then reset */
	if (i++ > 10000) 
	    i=0;
#ifdef SERVER
        // Update then multicast a random object
        index = rand() %10000;
        write_object(tem, index);
        write_socket(sockfd, (void *)&tem[index], sizeof(tzTEM_IO));
#else
	tzTEM_IO local_tem;
        read_socket(sockfd, (void *)&local_tem, sizeof(local_tem));
	index = local_tem.ObjectIndex;
	tem[index] = local_tem;
#endif
	/* Print out one object out of 10,000, and then sleep */
	if (i % 10000 == 0) 
  	{ 
	    read_object(tem, index);
     	    sleep(1);
	}
    }
 
    detach_and_destroy_shared_mem(tem, shmid);
    return 0; 
} 
