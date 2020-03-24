#include <stdio.h>

int main(int argc, char **argv) {
	/* Byte for degree, 2 bytes (short) for decimal part */
	char buf[6];
	unsigned int lat = 341958;
	unsigned int lon = 1197645;

	/*
	unsigned int byte_1 = (1<<8) -1;
	unsigned int byte_2 = ((1<<16) - 1) - byte_1;

	unsigned int lat_degrees = lat;
	unsigned int lon_degrees = lon;

	while(lat_degrees > 90) {
		lat_degrees /= 10;
	}

	while(lon_degrees > 180) {
		lon_degrees /= 10;
	}

	unsigned int lat_digits = lat - lat_degrees;
	unsigned int lon_digits = lon - lon_degrees;
	*/

	printf("%d 0x%x\n", lon, lon);

	buf[2] = lon & 0xFF;
	buf[1] = (lon & 0xFF00) >> 8;
	buf[0] = (lon & 0xFF0000) >> 16;
	buf[3] = (lat & 0xFF0000) >> 16;
	buf[4] = (lat & 0xFF00) >> 8;
	buf[5] = lat & 0xFF;

	FILE *write_ptr;
	write_ptr = fopen("test.bin","wb"); 
	fwrite(buf,6,1,write_ptr);
}
