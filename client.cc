/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/err.h>
#include <l4/sys/types.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/cxx/ipc_stream>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "shared.h"
#include <l4/dm_mem/dm_mem.h>
//#include <l4/l4rm/l4rm.h>

static void encrypt(L4::Cap<void> const &server)
   
{

  L4::Ipc::Iostream s(l4_utcb());
  s << l4_umword_t(Opcode::encrypt);
  int r = l4_error(s.call(server.cap(), Protocol::Encr));
  if (r)
    return NULL; // failure
}

static void decrypt(L4::Cap<void> const &server)
{

  L4::Ipc::Iostream s(l4_utcb());
  s << l4_umword_t(Opcode::decrypt)
  int r = l4_error(s.call(server.cap(), Protocol::Encr));
  if (r)
    return NULL; // failure
}
int send_dataspace(L4::Cap<void> const &server, l4dm_dataspace_t ds, unsigned long size)
{
	L4::Ipc::Iostream s(l4_utcb());
	s << l4_umword_t(Opcode::dataspace) <<ds<<size;
	int r = l4_error(s.call(server.cap(), Protocol::Encr));
	if (r)
    		return 1; // failure
         return 0;
}
int main()
   {
  	L4::Cap<void> server = L4Re::Env::env()->get_cap<void>("encr_server");
	if (!server.is_valid())
	{
     		 printf("Could not get server capability!\n");
     		 return 1;
    	}
	char text[]="Hello World";
	unsigned long size = 12;//len+/0

	l4dm_dataspace_t ds;
	void* addr;
	l4dm_mem_open(L4DM_DEFAULT_DSM,size,0,0,"EncrText",&ds);
	l4rm_attach(&ds,size,0,L4DM_RW,&addr);

	send_dataspace(server,ds,size);  
	memcpy(addr,text,size);
	printf(" %s \n",(char*)addr);
	
	if (!encrypt(server))	
	{
      		printf("Error talking to server\n");
      		return 1;
    	}
  	printf("Encrypted text: %s \n", (char*)addr);
	
	
	send_dataspace(server,ds,size); 
  	if (!decrypt(server))
  	{
         	printf("Error talking to server\n");
     		return 1;
        }
	printf("Decrypted text: %s \n",  (char*)addr);

	l4rm_detach(addr);
	l4dm_close(&ds);
	
	
  return 0;
 
}
