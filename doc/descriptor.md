# descriptor
```
typedef struct descriptor
{
	uint8_t tag;
	uint8_t length;
	struct list_node n;
	uint8_t data[0];
} descriptor_t;
```

A basic descriptor should have a structure with TLV like above.
A descriptor is a list node. As descriptors can be chained in a table.

Define sevaral MACROs as helpers to expand the struct, parse the ts sections,
dump the result and free the list.
For example, this expands definitions of a struct:
```
#define __m(type, name, bits) type name : bits; /* define the struct member with bits width */
#define __m1(type, name) type name;				/* define the struct member, which consume a type width */
```
And this define how to parse the struct member:
```
#define __m(type, name, bits)                                                                 \
	dr->name = TS_READ_BITS_##type(buf + bytes_off, bits, bits_off);                          \
	bits_off += bits;                                                                         \
	if (bits_off == sizeof(type) * 8) {                                                       \
		bits_off = 0;                                                                         \
		bytes_off += sizeof(type);                                                            \
	}

#define __m1(type, name)                                                                      \
	if (bytes_off < len) {                                                                    \
		dr->name = TS_READ_##type(buf + bytes_off);                                           \
		bytes_off += sizeof(type); }
```
We should implement macros for free and dump too.
Every macro should be well defined for the four helper types.
see ```include/descriptor.h```

Also, not every sub-structrue of a descriptor is defined as above. A simple TLV is not enough,
macro define could be too compilated.
If we do have many descriptors share the same pattern, we can do things like below
```
define __mplast_custom_sub(type, name, parser_cb, dump_cb, free_cb)
```
we can add all parse, dump and free functions as callback here.

If we don't have many structures share the same pattern, just write functions for it.
And add initialization for descriptors in ```init_descriptor_parsers``` function.
