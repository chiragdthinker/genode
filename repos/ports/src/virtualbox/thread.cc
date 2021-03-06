/*
 * \brief  Virtualbox adjusted pthread_create implementation
 * \author Alexander Boettcher
 * \date   2014-04-09
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* Genode */
#include <base/printf.h>
#include <base/thread.h>
#include <base/env.h>
#include <cpu_session/connection.h>

/* Genode libc pthread binding */
#include "thread.h"

#include "sup.h"

/* libc */
#include <pthread.h>
#include <errno.h>

/* vbox */
#include <internal/thread.h>

static Genode::Cpu_session * get_cpu_session(RTTHREADTYPE type) {
	using namespace Genode;

	static Cpu_connection * con[RTTHREADTYPE_END - 1];
	static Lock lock;

	Assert(type && type < RTTHREADTYPE_END);

	Lock::Guard guard(lock);

	if (con[type - 1])
		return con[type - 1];

	unsigned const VIRTUAL_GENODE_VBOX_LEVELS = 16;
	static_assert (RTTHREADTYPE_END < VIRTUAL_GENODE_VBOX_LEVELS,
	               "prio levels exceeds VIRTUAL_GENODE_VBOX_LEVELS");

	long const prio = (VIRTUAL_GENODE_VBOX_LEVELS - type) *
	                  Cpu_session::PRIORITY_LIMIT / VIRTUAL_GENODE_VBOX_LEVELS;

	char * data = new (env()->heap()) char[16];

	snprintf(data, 16, "vbox %u", type);

	con[type - 1] = new (env()->heap()) Cpu_connection(data, prio);

	/* upgrade memory of cpu session for frequent used thread type */
	if (type == RTTHREADTYPE_IO)
		Genode::env()->parent()->upgrade(con[type - 1]->cap(), "ram_quota=16384");

	return con[type - 1];
}


extern "C" int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
	                          void *(*start_routine) (void *), void *arg)
{
	PRTTHREADINT rtthread = reinterpret_cast<PRTTHREADINT>(arg);

	Assert(rtthread);

	size_t stack_size = Genode::Native_config::context_virtual_size() -
	                    sizeof(Genode::Native_utcb) - 2 * (1UL << 12);

	if (rtthread->cbStack < stack_size)
		stack_size = rtthread->cbStack;
	else
		PWRN("requested stack for thread '%s' of %zu Bytes is too large, "
		     "limit to %zu Bytes", rtthread->szName, rtthread->cbStack,
		     stack_size);

	/* sanity check - emt and vcpu thread have to have same prio class */
	if (!Genode::strcmp(rtthread->szName, "EMT"))
		Assert(rtthread->enmType == RTTHREADTYPE_EMULATION);

	if (rtthread->enmType == RTTHREADTYPE_EMULATION) {
		Genode::Cpu_session * cpu_session = get_cpu_session(RTTHREADTYPE_EMULATION);
/*
		Genode::Affinity::Space cpu_space = cpu_session->affinity_space();
		Genode::Affinity::Location location = cpu_space.location_of_index(i);
*/
		Genode::Affinity::Location location;
		if (create_emt_vcpu(thread, stack_size, attr, start_routine, arg,
		                    cpu_session, location))
			return 0;
		/* no haredware support, create normal pthread thread */
	}

	pthread_t thread_obj = new (Genode::env()->heap())
	                           pthread(attr ? *attr : 0, start_routine,
	                           arg, stack_size, rtthread->szName,
	                           get_cpu_session(rtthread->enmType));

	if (!thread_obj)
		return EAGAIN;

	*thread = thread_obj;

	thread_obj->start();

	return 0;
}

extern "C" int pthread_attr_setdetachstate(pthread_attr_t *, int)
{
	return 0;
}

extern "C" int pthread_attr_setstacksize(pthread_attr_t *, size_t)
{
	return 0;
}

extern "C" int pthread_atfork(void (*)(void), void (*)(void), void (*)(void))
{
	return 0;
}
