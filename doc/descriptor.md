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

Deine sevaral MACROs as helpers to expand the struct, parse the ts sections,
dump the result and free the list.
```
__m(type, member, bits)
__m1(type, member)
__mplast(type, member)
```
