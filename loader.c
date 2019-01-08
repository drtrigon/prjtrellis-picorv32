#define LED (*(volatile uint32_t*)0x02000000)

#define reg_uart_clkdiv (*(volatile uint32_t*)0x02000004)
#define reg_uart_data (*(volatile uint32_t*)0x02000008)

int getchar_available()
{
  return reg_uart_data & 0x100;
}

char getchar()
{
  return reg_uart_data;
}

void putchar(char c)
{
    if (c == '\n')
      putchar('\r');
    reg_uart_data = c;
}

void
sio_boot(void)
{
	int c, cnt, pos, val, len;
	char *cp;
	void *base_addr = NULL;

	/* Appease gcc's uninitialized variable warnings */
	val = 0;

	cp = NULL;	/* shut up uninitialized use warnings */

prompt:
	pos = 0;

	c = 0x76720A0D;			/* "\r\nrv" */
	do {
		putchar(c);
		c >>= 8;
		if (c == 0 && pos == 0) {
			pos = -1;
			c = 0x203E3233;	/* "32> " */
		}
	} while (c != 0);

next:
	pos = -1;
	len = 255;
	cnt = 2;

loop:
	/* Blink LEDs while waiting for serial input */
	do {
		if (pos < 0) {
			// RDTSC(val);
			if (val & 0x08000000)
				c = 0xff;
			else
				c = 0;
			if ((val & 0xff) > ((val >> 19) & 0xff))
				LED = c ^ 0x0f;
			else
				LED = c ^ 0xf0;
		} else
			LED = (int) cp >> 8;
		c = getchar_available();
	} while (c == 0);
	c = getchar();

	if (pos < 0) {
		if (c == 'S')
			pos = 0;
		else {
			if (c == '\r') /* CR ? */
				goto prompt;
			/* Echo char */
			if (c >= 32)
				pchar(c);
		}
		val = 0;
		goto loop;
	}
	if (c >= 10 && c <= 13) /* CR / LF ? */
		goto next;

	val <<= 4;
	if (c >= 'a')
		c -= 32;
	if (c >= 'A')
		val |= c - 'A' + 10;
	else
		val |= c - '0';
	pos++;

	/* Address width */
	if (pos == 1) {
		if (val >= 7 && val <= 9) {
			__asm __volatile__(
			"lui s0, 0x8000;"	/* stack mask */
			"lui s1, 0x1000;"	/* top of the initial stack */
			"and sp, %0, s0;"	/* clr low bits of the stack */
			"or sp, sp, s1;"	/* set stack */
			"mv ra, zero;"	
			"jr %0;"
			: 
			: "r" (base_addr)
			);
		}
		if (val <= 3)
			len = (val << 1) + 5;
		val = 0;
		goto loop;
	}

	/* Byte count */
	if (pos == 3) {
		cnt += (val << 1);
		val = 0;
		goto loop;
	}

	/* Valid len? */
	if (len < 6)
		goto loop;

	/* End of address */
	if (pos == len) {
		cp = (char *) val;
		if (base_addr == NULL)
			base_addr = (void *) val;
		goto loop;
	}

	if (pos > len && (pos & 1) && pos < cnt)
		*cp++ = val;

	goto loop;
	/* Unreached */
}
