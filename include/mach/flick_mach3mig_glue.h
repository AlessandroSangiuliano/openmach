/* 
 * Copyright (c) 1995 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 *      Author: Bryan Ford, University of Utah CSL
 */
/*
 * Glue for the Flick's Mach 3 backend.  (Flick := Flexible IDL Compiler Kit.)
 * This file is included from every stub source code (.c) file generated by that backend.
 * Stubs are built primarily out of invocations of these macros.
 */
#ifndef _MACH_FLICK_MACH3MIG_GLUE_H_
#define _MACH_FLICK_MACH3MIG_GLUE_H_

#include <stdlib.h>
#include <string.h>
#include <mach/mig_errors.h>
#include <mach/flick_mach3mig.h>


#define FLICK_NO_MEMORY		499 /*XXX*/


/*** Internal Flick data types ***/

/* Each client stub allocates one of these on its stack first thing,
   and holds all the important generic state throughout RPC processing.  */
struct flick_mach3mig_rpc_desc
{
	/* This is initially set to point to init_buf,
	   but is dynamically re-allocated if more space is needed.  */
	mig_reply_header_t *msg_buf;
	vm_size_t msg_buf_size;

	/* Before calling flick_mach3mig_rpc(),
	   the client stub sets this to the offset of the end of the data it marshaled.
	   It always starts marshaling just after the Mach message header.  */
	vm_size_t send_end_ofs;

	/* flick_mach3mig_rpc() sets these to the offset of the data to unmarshal,
	   and the offset of the end of the data to unmarshal, respectively.  */
	vm_size_t rcv_ofs, rcv_end_ofs;

	/* The actual size of this buffer varies from stub to stub.  */
	mig_reply_header_t init_buf;
};

/* Each server stub allocates one of these on its stack first thing,
   and holds all the important generic state throughout RPC processing.  */
struct flick_mach3mig_rpc_serv_desc
{
	/* During decoding msg_buf is InHeadP;
	   during encoding msg_buf is OutHeadP.
	   msg_buf_size is always simply a "very large" constant -
	   i.e. we don't know how big the buffer is; we just assume it's big enough.  */
	mig_reply_header_t *msg_buf;
	vm_size_t msg_buf_size;

	/* flick_mach3mig_serv_start_encode() sets these
	   to the offset of the data to unmarshal,
	   and the offset of the end of the data to unmarshal, respectively.  */
	vm_size_t rcv_ofs, rcv_end_ofs;

	/* After the reply message has been encoded,
	   this contains the offset of the end of the data it marshaled.  */
	vm_size_t send_end_ofs;
};



/*** Memory allocation/deallocation ***/

#define flick_alloc_mach_vm(size)							\
({											\
	vm_address_t addr;								\
	if (_err = vm_allocate(mach_task_self(), &addr, (size), 1)) return _err;	\
	(void*)addr;									\
})
#define flick_free_mach_vm(addr, size)							\
	if (_err = vm_deallocate(mach_task_self(), (addr), (size))) return _err;


/*** Encoding ***/

#define flick_mach3mig_encode_target(_data, _adjust)					\
{											\
	if (_adjust > 1) {								\
		if (_err = mach_port_mod_refs(mach_task_self(), (_data),		\
			MACH_PORT_RIGHT_SEND, -(_adjust-1))) return _err;		\
	}										\
	_desc.d.msg_buf->Head.msgh_remote_port = (_data);				\
	_desc.d.msg_buf->Head.msgh_bits = MACH_MSGH_BITS(				\
		_adjust ? MACH_MSG_TYPE_MOVE_SEND : MACH_MSG_TYPE_COPY_SEND, 0);	\
}

/* Primitive types with individual type descriptors.  */
#define flick_mach3mig_encode_new_glob(max_size)					\
{											\
	while (_desc.d.send_end_ofs + (max_size) > _desc.d.msg_buf_size)		\
		if (_err = flick_mach3mig_rpc_grow_buf(&_desc)) return _err;		\
	_e_chunk = (void*)_desc.d.msg_buf + _desc.d.send_end_ofs;			\
}
#define flick_mach3mig_encode_end_glob(max_size)					\
	_desc.d.send_end_ofs += (max_size);

#define flick_mach3mig_encode_new_chunk(size)	/* do nothing */
#define flick_mach3mig_encode_end_chunk(size)	(_e_chunk += (size))

#define flick_mach3mig_encode_prim(_ofs, _data, _name, _bits, _ctype)			\
{											\
	struct { mach_msg_type_t _t; _ctype _v; } *_p = (void*)(_e_chunk + _ofs);	\
	mach_msg_type_t _tmpl = { _name, _bits, 1, 1, 0, 0 };				\
	_p->_t = _tmpl; _p->_v = (_data);						\
}
#define flick_mach3mig_encode_boolean(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_BOOLEAN, 32, signed32_t);
#define flick_mach3mig_encode_char8(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_CHAR, 8, signed8_t);
#define flick_mach3mig_encode_char16(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 8, signed16_t);
#define flick_mach3mig_encode_signed8(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_8, 8, signed8_t);
#define flick_mach3mig_encode_unsigned8(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_8, 8, unsigned8_t);
#define flick_mach3mig_encode_signed16(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 16, signed16_t);
#define flick_mach3mig_encode_unsigned16(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 16, unsigned16_t);
#define flick_mach3mig_encode_signed32(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_32, 32, signed32_t);
#define flick_mach3mig_encode_unsigned32(_ofs, _data)	\
	flick_mach3mig_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_32, 32, unsigned32_t);
#define flick_mach3mig_encode_port(_ofs, _data, _adjust)				\
{											\
	if (_adjust > 1) {								\
		if (_err = mach_port_mod_refs(mach_task_self(), (_data),		\
			MACH_PORT_RIGHT_SEND, -(_adjust-1))) return _err;		\
	}										\
	flick_mach3mig_encode_prim(_ofs, _data,						\
		_adjust ? MACH_MSG_TYPE_MOVE_SEND : MACH_MSG_TYPE_COPY_SEND,		\
		32, mach_port_t);							\
}

/* Array type descriptors.  */
#define flick_mach3mig_array_encode_type(_ofs, _name, _bits, _ctype, _num, _inl) 	\
{											\
	mach_msg_type_t *_p = (void*)(_e_chunk + _ofs);					\
	mach_msg_type_t _tmpl = { _name, _bits, _num, _inl, 0, 0 };			\
	*_p = _tmpl;									\
}
#define flick_mach3mig_array_encode_long_type(_ofs, _name, _bits, _ctype, _num, _inl) 	\
{											\
	mach_msg_type_long_t *_p = (void*)(_e_chunk + _ofs);				\
	mach_msg_type_long_t _tmpl = { { 0, 0, 0, _inl, 1, 0 }, _name, _bits, _num};	\
	*_p = _tmpl;									\
}
#define flick_mach3mig_array_encode_boolean_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_BOOLEAN, 32, signed32_t, _num, _inl);
#define flick_mach3mig_array_encode_char8_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_CHAR, 8, signed8_t, _num, _inl);
#define flick_mach3mig_array_encode_char16_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_INTEGER_16, 8, signed16_t, _num, _inl);
#define flick_mach3mig_array_encode_signed8_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_INTEGER_8, 8, signed8_t, _num, _inl);
#define flick_mach3mig_array_encode_unsigned8_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_INTEGER_8, 8, unsigned8_t, _num, _inl);
#define flick_mach3mig_array_encode_signed16_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_INTEGER_16, 16, signed16_t, _num, _inl);
#define flick_mach3mig_array_encode_unsigned16_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_INTEGER_16, 16, unsigned16_t, _num, _inl);
#define flick_mach3mig_array_encode_signed32_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_INTEGER_32, 32, signed32_t, _num, _inl);
#define flick_mach3mig_array_encode_unsigned32_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_encode##_long(_ofs, MACH_MSG_TYPE_INTEGER_32, 32, unsigned32_t, _num, _inl);

/* Array elements.  */
#define flick_mach3mig_array_encode_new_glob(max_size)	flick_mach3mig_encode_new_glob(max_size)
#define flick_mach3mig_array_encode_end_glob(max_size)	flick_mach3mig_encode_end_glob(max_size)
#define flick_mach3mig_array_encode_new_chunk(size)	flick_mach3mig_encode_new_chunk(size)
#define flick_mach3mig_array_encode_end_chunk(size)	flick_mach3mig_encode_end_chunk(size)

#define flick_mach3mig_array_encode_prim(_ofs, _data, _name, _bits, _ctype)		\
{											\
	_ctype *_p = (void*)(_e_chunk + _ofs);						\
	*_p = (_data);									\
}
#define flick_mach3mig_array_encode_boolean(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_BOOLEAN, 32, signed32_t);
#define flick_mach3mig_array_encode_char8(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_CHAR, 8, signed8_t);
#define flick_mach3mig_array_encode_char16(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 8, signed16_t);
#define flick_mach3mig_array_encode_signed8(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_8, 8, signed8_t);
#define flick_mach3mig_array_encode_unsigned8(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_8, 8, unsigned8_t);
#define flick_mach3mig_array_encode_signed16(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 16, signed16_t);
#define flick_mach3mig_array_encode_unsigned16(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 16, unsigned16_t);
#define flick_mach3mig_array_encode_signed32(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_32, 32, signed32_t);
#define flick_mach3mig_array_encode_unsigned32(_ofs, _data)	\
	flick_mach3mig_array_encode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_32, 32, unsigned32_t);
#define flick_mach3mig_array_encode_port(_ofs, _data, _adjust)				\
{											\
	if (_adjust > 1) {								\
		if (_err = mach_port_mod_refs(mach_task_self(), (_data),		\
			MACH_PORT_RIGHT_SEND, -(_adjust-1))) return _err;		\
	}										\
	flick_mach3mig_array_encode_prim(_ofs, _data,					\
		_adjust ? MACH_MSG_TYPE_MOVE_SEND : MACH_MSG_TYPE_COPY_SEND,		\
		32, mach_port_t);							\
}

/* Out-of-line buffer support.  */
#define flick_mach3mig_array_encode_ool_start(_ofs, _size)				\
{											\
	vm_address_t *_p = (void*)(_e_chunk + _ofs);					\
	struct { struct { void *msg_buf; vm_size_t msg_buf_size, send_end_ofs; } d; } _desc; \
	void *_e_chunk;									\
											\
	_desc.d.msg_buf_size = (_size); 						\
	if (_err = vm_allocate(mach_task_self(), _p, _desc.d.msg_buf_size, 1))		\
		return _err;								\
	_desc.d.msg_buf = (void*)*_p; _desc.d.send_end_ofs = 0;

#define flick_mach3mig_array_encode_ool_end()						\
}



/*** Decoding ***/

#if TypeCheck
#define flick_iftypecheck(code) code
#else
#define flick_iftypecheck(code)
#endif

/* Primitive types with individual type descriptors.  */
#define flick_mach3mig_decode_new_glob(max_size)
#define flick_mach3mig_decode_end_glob(max_size)

#define flick_mach3mig_decode_new_chunk(size)						\
{											\
	flick_iftypecheck(								\
		if (_desc.d.rcv_ofs + (size) > _desc.d.rcv_end_ofs)			\
			return MIG_TYPE_ERROR;						\
	);										\
	_d_chunk = (void*)_desc.d.msg_buf + _desc.d.rcv_ofs;				\
}
#define flick_mach3mig_decode_end_chunk(size)						\
	_desc.d.rcv_ofs += (size);

#define flick_mach3mig_decode_prim(_ofs, _data, _name, _bits, _ctype)			\
{											\
	struct { mach_msg_type_t _t; _ctype _v; } *_p = (void*)(_d_chunk + _ofs);	\
	flick_iftypecheck( ({								\
		mach_msg_type_t _tmpl = { _name, _bits, 1, 1, 0, 0 };			\
		if (*((signed32_t*)&_tmpl) != *((signed32_t*)&_p->_t))			\
			return MIG_TYPE_ERROR;						\
	)} )										\
	(_data) = _p->_v;								\
}
#define flick_mach3mig_decode_boolean(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_BOOLEAN, 32, signed32_t);
#define flick_mach3mig_decode_char8(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_CHAR, 8, signed8_t);
#define flick_mach3mig_decode_char16(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 8, signed16_t);
#define flick_mach3mig_decode_signed8(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_8, 8, signed8_t);
#define flick_mach3mig_decode_unsigned8(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_8, 8, unsigned8_t);
#define flick_mach3mig_decode_signed16(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 16, signed16_t);
#define flick_mach3mig_decode_unsigned16(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 16, unsigned16_t);
#define flick_mach3mig_decode_signed32(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_32, 32, signed32_t);
#define flick_mach3mig_decode_unsigned32(_ofs, _data)	\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_32, 32, unsigned32_t);
#define flick_mach3mig_decode_port(_ofs, _data, _adjust)				\
{											\
	flick_mach3mig_decode_prim(_ofs, _data, MACH_MSG_TYPE_PORT_SEND, 32, mach_port_t);	\
	if (_adjust != 1) {								\
		if (_err = mach_port_mod_refs(mach_task_self(), (_data),		\
			MACH_PORT_RIGHT_SEND, _adjust-1)) return _err;			\
	}										\
}

/* Array type descriptors.  */
#define flick_mach3mig_array_decode_type(_ofs, _name, _bits, _ctype, _num, _inl) 	\
{											\
	mach_msg_type_t *_p = (void*)(_e_chunk + _ofs);					\
	flick_iftypecheck( ({								\
		mach_msg_type_t _tmpl = { _name, _bits, _num, _inl, 0, 0 };		\
		if (*((signed32_t*)&_tmpl) != *((signed32_t*)_p))			\
			return MIG_TYPE_ERROR;						\
	)} )										\
}
#define flick_mach3mig_array_decode_long_type(_ofs, _name, _bits, _ctype, _num, _inl) 	\
{											\
	mach_msg_type_long_t *_p = (void*)(_e_chunk + _ofs);				\
	flick_iftypecheck( ({								\
		mach_msg_type_long_t _tmpl = { { 0, 0, 0, _inl, 1, 0 }, _name, _bits, _num}; \
		if (memcmp(&_tmpl, _p, sizeof(_tmpl)))					\
			return MIG_TYPE_ERROR;						\
	)} )										\
}
#define flick_mach3mig_array_decode_boolean_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_BOOLEAN, 32, signed32_t, _num, _inl);
#define flick_mach3mig_array_decode_char8_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_CHAR, 8, signed8_t, _num, _inl);
#define flick_mach3mig_array_decode_char16_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_INTEGER_16, 8, signed16_t, _num, _inl);
#define flick_mach3mig_array_decode_signed8_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_INTEGER_8, 8, signed8_t, _num, _inl);
#define flick_mach3mig_array_decode_unsigned8_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_INTEGER_8, 8, unsigned8_t, _num, _inl);
#define flick_mach3mig_array_decode_signed16_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_INTEGER_16, 16, signed16_t, _num, _inl);
#define flick_mach3mig_array_decode_unsigned16_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_INTEGER_16, 16, unsigned16_t, _num, _inl);
#define flick_mach3mig_array_decode_signed32_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_INTEGER_32, 32, signed32_t, _num, _inl);
#define flick_mach3mig_array_decode_unsigned32_type(_ofs, _num, _inl, _long)	\
	flick_mach3mig_array_decode##_long(_ofs, MACH_MSG_TYPE_INTEGER_32, 32, unsigned32_t, _num, _inl);

/* Array elements.  */
#define flick_mach3mig_array_decode_new_glob(max_size)	flick_mach3mig_decode_new_glob(max_size)
#define flick_mach3mig_array_decode_end_glob(max_size)	flick_mach3mig_decode_end_glob(max_size)
#define flick_mach3mig_array_decode_new_chunk(size)	flick_mach3mig_decode_new_chunk(size)
#define flick_mach3mig_array_decode_end_chunk(size)	flick_mach3mig_decode_end_chunk(size)

#define flick_mach3mig_array_decode_prim(_ofs, _data, _name, _bits, _ctype)		\
{											\
	_ctype *_p = (void*)(_d_chunk + _ofs);						\
	(_data) = *_p;									\
}
#define flick_mach3mig_array_decode_boolean(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_BOOLEAN, 32, signed32_t);
#define flick_mach3mig_array_decode_char8(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_CHAR, 8, signed8_t);
#define flick_mach3mig_array_decode_char16(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 8, signed16_t);
#define flick_mach3mig_array_decode_signed8(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_8, 8, signed8_t);
#define flick_mach3mig_array_decode_unsigned8(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_8, 8, unsigned8_t);
#define flick_mach3mig_array_decode_signed16(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 16, signed16_t);
#define flick_mach3mig_array_decode_unsigned16(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_16, 16, unsigned16_t);
#define flick_mach3mig_array_decode_signed32(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_32, 32, signed32_t);
#define flick_mach3mig_array_decode_unsigned32(_ofs, _data)	\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_INTEGER_32, 32, unsigned32_t);
#define flick_mach3mig_array_decode_port(_ofs, _data, _adjust)				\
{											\
	flick_mach3mig_array_decode_prim(_ofs, _data, MACH_MSG_TYPE_PORT_SEND, 32, mach_port_t); \
	if (_adjust != 1) {								\
		kern_return_t res = mach_port_mod_refs(mach_task_self(), (_data),	\
			MACH_PORT_RIGHT_SEND, _adjust-1);				\
	}										\
}

/* Out-of-line buffer support.  */
#define flick_mach3mig_array_decode_ool_start(_ofs, _size)				\
{											\
	vm_address_t *_p = (void*)(_e_chunk + _ofs);					\
	struct { struct { void *msg_buf; vm_size_t rcv_ofs, rcv_end_ofs; } d; } _desc;	\
	void *_e_chunk;									\
											\
	_desc.d.msg_buf = (void*)*_p; _desc.d.rcv_ofs = 0; _desc.d.rcv_end_ofs = (_size);\

#define flick_mach3mig_array_decode_ool_end()						\
	if (_err = vm_deallocate(mach_task_self(), *_p, _desc.d.rcv_end_ofs))		\
		return _err;								\
}


/*** Client-side support ***/

mach_msg_return_t flick_mach3mig_rpc(struct flick_mach3mig_rpc_desc *rpc);

#define flick_mach3mig_rpc_macro(iscomplex)						\
{											\
	_desc.d.msg_buf->Head.msgh_bits |=						\
		MACH_MSGH_BITS(0, MACH_MSG_TYPE_MAKE_SEND_ONCE)				\
		| (iscomplex ? MACH_MSGH_BITS_COMPLEX : 0);				\
	if (_err = flick_mach3mig_rpc(&_desc.d)) return _err;				\
}

#define flick_mach3mig_send_macro(iscomplex)						\
{											\
	_desc.d.msg_buf->Head.msgh_bits |= (iscomplex ? MACH_MSGH_BITS_COMPLEX : 0);	\
	if (_err = flick_mach3mig_rpc(&_desc.d)) return _err;				\
}


/*** Server-side support ***/

#define flick_mach3mig_serv_start_decode()						\
{											\
	_desc.d.msg_buf = (mig_reply_header_t*)InHeadP;					\
	_desc.d.msg_buf_size = 0x7fffffff; 						\
	_desc.d.rcv_ofs = sizeof(mach_msg_header_t);					\
	_desc.d.rcv_end_ofs = InHeadP->msgh_size;					\
}

#define flick_mach3mig_serv_end_decode() /* do nothing */

#define flick_mach3mig_serv_start_encode()						\
{											\
	_desc.d.msg_buf = (mig_reply_header_t*)OutHeadP;				\
	_desc.d.send_end_ofs = sizeof(mig_reply_header_t);				\
}

#define flick_mach3mig_serv_end_encode()						\
{											\
	mach_msg_type_t _ret_tmpl = { MACH_MSG_TYPE_INTEGER_32, 32, 1, 1, 0, 0 };	\
	OutHeadP->msgh_bits = MACH_MSGH_BITS(MACH_MSGH_BITS_REPLY(InHeadP->msgh_bits), 0); \
	OutHeadP->msgh_size = _desc.d.send_end_ofs;					\
	OutHeadP->msgh_remote_port = InHeadP->msgh_remote_port;				\
	OutHeadP->msgh_local_port = MACH_PORT_NULL;					\
        _desc.d.msg_buf->RetCodeType = _ret_tmpl;					\
	_desc.d.msg_buf->RetCode = _return;						\
}


#endif /* _MACH_FLICK_MACH3MIG_GLUE_H_ */
