#ifndef _PIC_H_
#define _PIC_H_

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

void PIC_Initialize();
void PIC_SetOffset(int off1, int off2);


#endif /* end of include guard: _PIC_H_ */
