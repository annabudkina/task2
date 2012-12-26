/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <stdio.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/cxx/ipc_server>
#include <l4/cxx/ipc_stream>
#include "shared.h"
#include <l4/dm_mem/dm_mem.h>
//#include <l4/l4rm/l4rm.h>


static L4Re::Util::Registry_server<> server;

class Encryption_server : public L4::Server_object
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
};

int Encryption_server::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
  l4dm_dataspace_t ds;
  void* addr;
  char*text=(char*)addr;
  unsigned long size;

  l4_msgtag_t t;
  ios >> t;

  if (t.label() != Protocol::Encr)
    return -L4_EBADPROTO;

  L4::Opcode opcode;
  ios >> opcode;
  switch (opcode)
    {
    case Opcode::encrypt:	
      for(unsigned i=0;i!=size-1;i++)
	{
		text[i]=text[i]+1;
	}

      l4rm_detach(addr);
      return L4_EOK;


    case Opcode::decrypt:
      for(unsigned i=0;i!=size-1;i++)
	{
		text[i]=text[i]-1;
	}

      l4rm_detach(addr);
      return L4_EOK;

	case Opcode::dataspace:
		ios>>ds;
		ios>>size;
		l4rm_attach(&ds,size,0,L4DM_RW,&addr);
		return L4_EOK;
	  
    default:
      return -L4_ENOSYS;
    }
}

int main()
{
  static Encryption_server encr;

  if (!server.registry()->register_obj(&encr, "encr_server").is_valid())
    {
      printf("Could not register my service, is there a 'encr_server' in the caps table?\n");
      return 1;
    }


  // Wait for client requests
  server.loop();

  return 0;
}

