
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "v210.h"

#define LOCAL_DEBUG 0

/* TODO: duplicate of font from osd.c */
static struct letter_t {
	unsigned char *ptr;
	unsigned char data[8];
} charset[] = 
{
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /*     */        { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* ' ' */          { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '!' */          { 0, { 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x04, 0x00 }, },
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000000 */
    /* 00000000 */
    /* 00000100 */
    /* 00000000 */
    /* '"' */          { 0, { 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00001010 */
    /* 00001010 */
    /* 00001010 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '#' */          { 0, { 0x0a, 0x0a, 0x1f, 0x0a, 0x1f, 0x0a, 0x0a, 0x00 }, },
    /* 00001010 */
    /* 00001010 */
    /* 00011111 */
    /* 00001010 */
    /* 00011111 */
    /* 00001010 */
    /* 00001010 */
    /* 00000000 */
    /* '$' */          { 0, { 0x04, 0x0f, 0x14, 0x0e, 0x05, 0x1e, 0x04, 0x00 }, },
    /* 00000100 */
    /* 00001111 */
    /* 00010100 */
    /* 00001110 */
    /* 00000101 */
    /* 00011110 */
    /* 00000100 */
    /* 00000000 */
    /* '%' */          { 0, { 0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03, 0x00 }, },
    /* 00011000 */
    /* 00011001 */
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00010011 */
    /* 00000011 */
    /* 00000000 */
    /* '&' */          { 0, { 0x0c, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0d, 0x00 }, },
    /* 00001100 */
    /* 00010010 */
    /* 00010100 */
    /* 00001000 */
    /* 00010101 */
    /* 00010010 */
    /* 00001101 */
    /* 00000000 */
    /* ''' */          { 0, { 0x0c, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00001100 */
    /* 00000100 */
    /* 00001000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '(' */          { 0, { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02, 0x00 }, },
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00001000 */
    /* 00001000 */
    /* 00000100 */
    /* 00000010 */
    /* 00000000 */
    /* ')' */          { 0, { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00 }, },
    /* 00001000 */
    /* 00000100 */
    /* 00000010 */
    /* 00000010 */
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00000000 */
    /* '*' */          { 0, { 0x00, 0x04, 0x15, 0x0e, 0x15, 0x04, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000100 */
    /* 00010101 */
    /* 00001110 */
    /* 00010101 */
    /* 00000100 */
    /* 00000000 */
    /* 00000000 */
    /* '+' */          { 0, { 0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000100 */
    /* 00000100 */
    /* 00011111 */
    /* 00000100 */
    /* 00000100 */
    /* 00000000 */
    /* 00000000 */
    /* ',' */          { 0, { 0x00, 0x00, 0x00, 0x00, 0x0c, 0x04, 0x08, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00001100 */
    /* 00000100 */
    /* 00001000 */
    /* 00000000 */
    /* '-' */          { 0, { 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00011111 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '.' */          { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00001100 */
    /* 00001100 */
    /* 00000000 */
    /* '/' */          { 0, { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000001 */
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00010000 */
    /* 00000000 */
    /* 00000000 */
    /* '0' */          { 0, { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010011 */
    /* 00010101 */
    /* 00011001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* '1' */          { 0, { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e, 0x00 }, },
    /* 00000100 */
    /* 00001100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00001110 */
    /* 00000000 */
    /* '2' */          { 0, { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00000001 */
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00011111 */
    /* 00000000 */
    /* '3' */          { 0, { 0x1f, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0e, 0x00 }, },
    /* 00011111 */
    /* 00000010 */
    /* 00000100 */
    /* 00000010 */
    /* 00000001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* '4' */          { 0, { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02, 0x00 }, },
    /* 00000010 */
    /* 00000110 */
    /* 00001010 */
    /* 00010010 */
    /* 00011111 */
    /* 00000010 */
    /* 00000010 */
    /* 00000000 */
    /* '5' */          { 0, { 0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e, 0x00 }, },
    /* 00011111 */
    /* 00010000 */
    /* 00011110 */
    /* 00000001 */
    /* 00000001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* '6' */          { 0, { 0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e, 0x00 }, },
    /* 00000110 */
    /* 00001000 */
    /* 00010000 */
    /* 00011110 */
    /* 00010001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* '7' */          { 0, { 0x1f, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x00 }, },
    /* 00011111 */
    /* 00000001 */
    /* 00000010 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000000 */
    /* '8' */          { 0, { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010001 */
    /* 00001110 */
    /* 00010001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* '9' */          { 0, { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010001 */
    /* 00001111 */
    /* 00000001 */
    /* 00000010 */
    /* 00001100 */
    /* 00000000 */
    /* ':' */          { 0, { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00001100 */
    /* 00001100 */
    /* 00000000 */
    /* 00001100 */
    /* 00001100 */
    /* 00000000 */
    /* 00000000 */
    /* ';' */          { 0, { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x04, 0x08, 0x00 }, },
    /* 00000000 */
    /* 00001100 */
    /* 00001100 */
    /* 00000000 */
    /* 00001100 */
    /* 00000100 */
    /* 00001000 */
    /* 00000000 */
    /* '<' */  { 0, { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00 }, },
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00010000 */
    /* 00001000 */
    /* 00000100 */
    /* 00000010 */
    /* 00000000 */
    /* '=' */  { 0, { 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00011111 */
    /* 00000000 */
    /* 00011111 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '>' */  { 0, { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00 }, },
    /* 00001000 */
    /* 00000100 */
    /* 00000010 */
    /* 00000001 */
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00000001 */
    /* 00000010 */
    /* 00000100 */
    /* 00000000 */
    /* 00000100 */
    /* 00000000 */
    /* '@' */  { 0, { 0x0e, 0x11, 0x01, 0x0d, 0x15, 0x15, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00000001 */
    /* 00001101 */
    /* 00010101 */
    /* 00010101 */
    /* 00001110 */
    /* 00000000 */
    /* 'A' */  { 0, { 0x0e, 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00011111 */
    /* 00010001 */
    /* 00010001 */
    /* 00000000 */
    /* 'B' */  { 0, { 0x1e, 0x09, 0x09, 0x0e, 0x09, 0x09, 0x1e, 0x00 }, },
    /* 00011110 */
    /* 00001001 */
    /* 00001001 */
    /* 00001110 */
    /* 00001001 */
    /* 00001001 */
    /* 00011110 */
    /* 00000000 */
    /* 'C' */  { 0, { 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010000 */
    /* 00010000 */
    /* 00010000 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* 'D' */  { 0, { 0x1e, 0x09, 0x09, 0x09, 0x09, 0x09, 0x1e, 0x00 }, },
    /* 00011110 */
    /* 00001001 */
    /* 00001001 */
    /* 00001001 */
    /* 00001001 */
    /* 00001001 */
    /* 00011110 */
    /* 00000000 */
    /* 'E' */  { 0, { 0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x1f, 0x00 }, },
    /* 00011111 */
    /* 00010000 */
    /* 00010000 */
    /* 00011111 */
    /* 00010000 */
    /* 00010000 */
    /* 00011111 */
    /* 00000000 */
    /* 'F' */  { 0, { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10, 0x00 }, },
    /* 00011111 */
    /* 00010000 */
    /* 00010000 */
    /* 00011110 */
    /* 00010000 */
    /* 00010000 */
    /* 00010000 */
    /* 00000000 */
    /* 'G' */  { 0, { 0x0e, 0x11, 0x10, 0x13, 0x11, 0x11, 0x0f, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010000 */
    /* 00010011 */
    /* 00010001 */
    /* 00010001 */
    /* 00001111 */
    /* 00000000 */
    /* 'H' */  { 0, { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11, 0x00 }, },
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00011111 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00000000 */
    /* 'I' */  { 0, { 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00001110 */
    /* 00000000 */
    /* 'J' */  { 0, { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c, 0x00 }, },
    /* 00000111 */
    /* 00000010 */
    /* 00000010 */
    /* 00000010 */
    /* 00000010 */
    /* 00010010 */
    /* 00001100 */
    /* 00000000 */
    /* 'K' */  { 0, { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11, 0x00 }, },
    /* 00010001 */
    /* 00010010 */
    /* 00010100 */
    /* 00011000 */
    /* 00010100 */
    /* 00010010 */
    /* 00010001 */
    /* 00000000 */
    /* 'L' */  { 0, { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x00 }, },
    /* 00010000 */
    /* 00010000 */
    /* 00010000 */
    /* 00010000 */
    /* 00010000 */
    /* 00010000 */
    /* 00011111 */
    /* 00000000 */
    /* 'M' */  { 0, { 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11, 0x00 }, },
    /* 00010001 */
    /* 00011011 */
    /* 00010101 */
    /* 00010101 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00000000 */
    /* 'N' */  { 0, { 0x11, 0x19, 0x19, 0x15, 0x13, 0x13, 0x11, 0x00 }, },
    /* 00010001 */
    /* 00011001 */
    /* 00011001 */
    /* 00010101 */
    /* 00010011 */
    /* 00010011 */
    /* 00010001 */
    /* 00000000 */
    /* 'O' */  { 0, { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* 'P' */  { 0, { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10, 0x00 }, },
    /* 00011110 */
    /* 00010001 */
    /* 00010001 */
    /* 00011110 */
    /* 00010000 */
    /* 00010000 */
    /* 00010000 */
    /* 00000000 */
    /* 'Q' */  { 0, { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x1d, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010101 */
    /* 00010010 */
    /* 00011101 */
    /* 00000000 */
    /* 'R' */  { 0, { 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11, 0x00 }, },
    /* 00011110 */
    /* 00010001 */
    /* 00010001 */
    /* 00011110 */
    /* 00010100 */
    /* 00010010 */
    /* 00010001 */
    /* 00000000 */
    /* 'S' */  { 0, { 0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00010001 */
    /* 00010000 */
    /* 00001110 */
    /* 00000001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* 'T' */  { 0, { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 }, },
    /* 00011111 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000000 */
    /* 'U' */  { 0, { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00 }, },
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* 'V' */  { 0, { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x00 }, },
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00001010 */
    /* 00000100 */
    /* 00000000 */
    /* 'W' */  { 0, { 0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11, 0x00 }, },
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010101 */
    /* 00010101 */
    /* 00011011 */
    /* 00010001 */
    /* 00000000 */
    /* 'X' */  { 0, { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11, 0x00 }, },
    /* 00010001 */
    /* 00010001 */
    /* 00001010 */
    /* 00000100 */
    /* 00001010 */
    /* 00010001 */
    /* 00010001 */
    /* 00000000 */
    /* 'Y' */  { 0, { 0x11, 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x00 }, },
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00001010 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000000 */
    /* 'Z' */  { 0, { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f, 0x00 }, },
    /* 00011111 */
    /* 00000001 */
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00010000 */
    /* 00011111 */
    /* 00000000 */
    /* '[' */  { 0, { 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00001000 */
    /* 00001000 */
    /* 00001000 */
    /* 00001000 */
    /* 00001000 */
    /* 00001110 */
    /* 00000000 */
    /* '\' */  { 0, { 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00 }, },
    /* 00010000 */
    /* 00001000 */
    /* 00000100 */
    /* 00000010 */
    /* 00000001 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* ']' */  { 0, { 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e, 0x00 }, },
    /* 00001110 */
    /* 00000010 */
    /* 00000010 */
    /* 00000010 */
    /* 00000010 */
    /* 00000010 */
    /* 00001110 */
    /* 00000000 */
    /* '^' */  { 0, { 0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000100 */
    /* 00001010 */
    /* 00010001 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '_' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00011111 */
    /* 00000000 */
    /* '`' */  { 0, { 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00010000 */
    /* 00001000 */
    /* 00000100 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 'a' */  { 0, { 0x00, 0x00, 0x0e, 0x01, 0x0f, 0x11, 0x0f, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00001110 */
    /* 00000001 */
    /* 00001111 */
    /* 00010001 */
    /* 00001111 */
    /* 00000000 */
    /* 'b' */  { 0, { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1e, 0x00 }, },
    /* 00010000 */
    /* 00010000 */
    /* 00010110 */
    /* 00011001 */
    /* 00010001 */
    /* 00010001 */
    /* 00011110 */
    /* 00000000 */
    /* 'c' */  { 0, { 0x00, 0x00, 0x0e, 0x11, 0x10, 0x11, 0x0e, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00001110 */
    /* 00010001 */
    /* 00010000 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* 'd' */  { 0, { 0x01, 0x01, 0x0d, 0x13, 0x11, 0x11, 0x0f, 0x00 }, },
    /* 00000001 */
    /* 00000001 */
    /* 00001101 */
    /* 00010011 */
    /* 00010001 */
    /* 00010001 */
    /* 00001111 */
    /* 00000000 */
    /* 'e' */  { 0, { 0x00, 0x00, 0x0e, 0x11, 0x1f, 0x10, 0x0e, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00001110 */
    /* 00010001 */
    /* 00011111 */
    /* 00010000 */
    /* 00001110 */
    /* 00000000 */
    /* 'f' */  { 0, { 0x02, 0x05, 0x04, 0x0e, 0x04, 0x04, 0x04, 0x00 }, },
    /* 00000010 */
    /* 00000101 */
    /* 00000100 */
    /* 00001110 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000000 */
    /* 'g' */  { 0, { 0x00, 0x0d, 0x13, 0x13, 0x0d, 0x01, 0x0e, 0x00 }, },
    /* 00000000 */
    /* 00001101 */
    /* 00010011 */
    /* 00010011 */
    /* 00001101 */
    /* 00000001 */
    /* 00001110 */
    /* 00000000 */
    /* 'h' */  { 0, { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00 }, },
    /* 00010000 */
    /* 00010000 */
    /* 00010110 */
    /* 00011001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00000000 */
    /* 'i' */  { 0, { 0x04, 0x00, 0x0c, 0x04, 0x04, 0x04, 0x0e, 0x00 }, },
    /* 00000100 */
    /* 00000000 */
    /* 00001100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00001110 */
    /* 00000000 */
    /* 'j' */  { 0, { 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0c, 0x00 }, },
    /* 00000010 */
    /* 00000000 */
    /* 00000110 */
    /* 00000010 */
    /* 00000010 */
    /* 00010010 */
    /* 00001100 */
    /* 00000000 */
    /* 'k' */  { 0, { 0x08, 0x08, 0x09, 0x0a, 0x0c, 0x0a, 0x09, 0x00 }, },
    /* 00001000 */
    /* 00001000 */
    /* 00001001 */
    /* 00001010 */
    /* 00001100 */
    /* 00001010 */
    /* 00001001 */
    /* 00000000 */
    /* 'l' */  { 0, { 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e, 0x00 }, },
    /* 00001100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00001110 */
    /* 00000000 */
    /* 'm' */  { 0, { 0x00, 0x00, 0x1a, 0x15, 0x15, 0x15, 0x15, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00011010 */
    /* 00010101 */
    /* 00010101 */
    /* 00010101 */
    /* 00010101 */
    /* 00000000 */
    /* 'n' */  { 0, { 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00010110 */
    /* 00011001 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00000000 */
    /* 'o' */  { 0, { 0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00001110 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00001110 */
    /* 00000000 */
    /* 'p' */  { 0, { 0x00, 0x16, 0x19, 0x19, 0x16, 0x10, 0x10, 0x00 }, },
    /* 00000000 */
    /* 00010110 */
    /* 00011001 */
    /* 00011001 */
    /* 00010110 */
    /* 00010000 */
    /* 00010000 */
    /* 00000000 */
    /* 'q' */  { 0, { 0x00, 0x0d, 0x13, 0x13, 0x0d, 0x01, 0x01, 0x00 }, },
    /* 00000000 */
    /* 00001101 */
    /* 00010011 */
    /* 00010011 */
    /* 00001101 */
    /* 00000001 */
    /* 00000001 */
    /* 00000000 */
    /* 'r' */  { 0, { 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00010110 */
    /* 00011001 */
    /* 00010000 */
    /* 00010000 */
    /* 00010000 */
    /* 00000000 */
    /* 's' */  { 0, { 0x00, 0x00, 0x0f, 0x10, 0x1e, 0x01, 0x1f, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00001111 */
    /* 00010000 */
    /* 00011110 */
    /* 00000001 */
    /* 00011111 */
    /* 00000000 */
    /* 't' */  { 0, { 0x08, 0x08, 0x1c, 0x08, 0x08, 0x09, 0x06, 0x00 }, },
    /* 00001000 */
    /* 00001000 */
    /* 00011100 */
    /* 00001000 */
    /* 00001000 */
    /* 00001001 */
    /* 00000110 */
    /* 00000000 */
    /* 'u' */  { 0, { 0x00, 0x00, 0x12, 0x12, 0x12, 0x12, 0x0d, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00010010 */
    /* 00010010 */
    /* 00010010 */
    /* 00010010 */
    /* 00001101 */
    /* 00000000 */
    /* 'v' */  { 0, { 0x00, 0x00, 0x11, 0x11, 0x11, 0x0a, 0x04, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00010001 */
    /* 00010001 */
    /* 00010001 */
    /* 00001010 */
    /* 00000100 */
    /* 00000000 */
    /* 'w' */  { 0, { 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0a, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00010001 */
    /* 00010001 */
    /* 00010101 */
    /* 00010101 */
    /* 00001010 */
    /* 00000000 */
    /* 'x' */  { 0, { 0x00, 0x00, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00010001 */
    /* 00001010 */
    /* 00000100 */
    /* 00001010 */
    /* 00010001 */
    /* 00000000 */
    /* 'y' */  { 0, { 0x00, 0x00, 0x11, 0x11, 0x13, 0x0d, 0x01, 0x0e }, },
    /* 00000000 */
    /* 00000000 */
    /* 00010001 */
    /* 00010001 */
    /* 00010011 */
    /* 00001101 */
    /* 00000001 */
    /* 00001110 */
    /* 'z' */  { 0, { 0x00, 0x00, 0x1f, 0x02, 0x04, 0x08, 0x1f, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00011111 */
    /* 00000010 */
    /* 00000100 */
    /* 00001000 */
    /* 00011111 */
    /* 00000000 */
    /* '{' */  { 0, { 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02, 0x00 }, },
    /* 00000010 */
    /* 00000100 */
    /* 00000100 */
    /* 00001000 */
    /* 00000100 */
    /* 00000100 */
    /* 00000010 */
    /* 00000000 */
    /* '|' */  { 0, { 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x04, 0x00 }, },
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000000 */
    /* 00000100 */
    /* 00000100 */
    /* 00000100 */
    /* 00000000 */
    /* '}' */  { 0, { 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08, 0x00 }, },
    /* 00001000 */
    /* 00000100 */
    /* 00000100 */
    /* 00000010 */
    /* 00000100 */
    /* 00000100 */
    /* 00001000 */
    /* 00000000 */
    /* '~' */  { 0, { 0x08, 0x15, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00001000 */
    /* 00010101 */
    /* 00000010 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* '?' */  { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
    /* 00000000 */
};


#define  y_white 0x3ff
#define  y_black 0x000
#define cr_white 0x200
#define cb_white 0x200

/* Six pixels */
uint32_t V210_white[] = {
	 cr_white << 20 |  y_white << 10 | cb_white,
	  y_white << 20 | cb_white << 10 |  y_white,
	 cb_white << 20 |  y_white << 10 | cr_white,
	  y_white << 20 | cr_white << 10 |  y_white,
};

uint32_t V210_black[] = {
	 cr_white << 20 |  y_black << 10 | cb_white,
	  y_black << 20 | cb_white << 10 |  y_black,
	 cb_white << 20 |  y_black << 10 | cr_white,
	  y_black << 20 | cr_white << 10 |  y_black,
};

/* KL paint 6 pixels in a single point */
__inline__ void V210_draw_6_pixels(uint32_t *addr, uint32_t *coloring)
{
	for (int i = 0; i < (V210_BOX_HEIGHT / 6); i++) {
		addr[0] = coloring[0];
		addr[1] = coloring[1];
		addr[2] = coloring[2];
		addr[3] = coloring[3];
		addr += 4;
	}
}

__inline__ void V210_draw_box(uint32_t *frame_addr, uint32_t stride, int color, int interlaced)
{
	uint32_t *coloring;
	if (color == 1)
		coloring = V210_white;
	else
		coloring = V210_black;

	int interleaved = interlaced ? 2 : 1;
	interleaved = 1;
	for (uint32_t l = 0; l < V210_BOX_HEIGHT; l++) {
		uint32_t *addr = frame_addr + ((l * interleaved) * (stride / 4));
		V210_draw_6_pixels(addr, coloring);
	}
}

__inline__ void V210_draw_box_at(uint32_t *frame_addr, uint32_t stride, int color, int x, int y, int interlaced)
{
	uint32_t *addr = frame_addr + (y * (stride / 4));
	addr += ((x / 6) * 4);
	V210_draw_box(addr, stride, color, interlaced);
}

void V210_write_32bit_value(void *frame_bytes, uint32_t stride, uint32_t value, uint32_t lineNr, int interlaced)
{
	for (int p = 31, sh = 0; p >= 0; p--, sh++) {
		V210_draw_box_at(((uint32_t *)frame_bytes), stride,
			(value & (1 << sh)) == (uint32_t)(1 << sh), p * V210_BOX_HEIGHT, lineNr, interlaced);
	}
}

uint32_t V210_read_32bit_value(void *frame_bytes, uint32_t stride, uint32_t lineNr, double scalefactor)
{
	double pixheight = V210_BOX_HEIGHT * scalefactor;
	double newlinenr = lineNr * scalefactor;

	int xpos = 0;
	uint32_t bits = 0;
	for (int i = 0; i < 32; i++) {
		xpos = (i * pixheight) + (pixheight / 2);
		/* Sample the pixel two lines deeper than the initial line, and eight pixels in from the left */
		uint32_t *addr = ((uint32_t *)frame_bytes) + (((int)newlinenr + 2) * (stride / 4));
		addr += ((xpos / 6) * 4);

		bits <<= 1;

		/* Sample the pixel.... Compressor will decimate, we'll need a luma threshold for production. */
		if ((addr[1] & 0x3ff) > 0x080)
			bits |= 1;
	}
#if LOCAL_DEBUG
	printf("%s(%p) = 0x%08x\n", __func__, frame_bytes, bits);
#endif
	return bits;
}

static int V210_render_character(struct V210_painter_s *ctx, unsigned char letter)
{
	unsigned char line;
    
	if (letter > 0x9f)
		return -1;

	/* Render a character six pixels per dot (so 6 * width)
	 * and reduce the vertical aspect ratio by rendering four lines for each single line in the character.
	 * Therefore a normal single pixel in the character is rendered as 6x4 pixels.
	 */
	for (int i = 0; i < 8; i++) { /* Row */
       
		for (int z = 0; z < 4; z++)
		{
			line = charset[ letter ].data[ i ];

			/* Save a little screen re-estate, the first two pixels
			 * are always blank. so, render the characters as 6x8 instead of 8x8.
			 */
			line <<= 2;
			for (int j = 0; j < 6; j++) { /* Col */
				if (line & 0x80) {
					/* font color */
					*(ctx->ptr + (j * 4) + 0) = V210_white[0];
					*(ctx->ptr + (j * 4) + 1) = V210_white[1];
					*(ctx->ptr + (j * 4) + 2) = V210_white[2];
					*(ctx->ptr + (j * 4) + 3) = V210_white[3];
				} else {
					/* background color */
					/* Complete black background */
					*(ctx->ptr + (j * 4) + 0) = V210_black[0];
					*(ctx->ptr + (j * 4) + 1) = V210_black[1];
					*(ctx->ptr + (j * 4) + 2) = V210_black[2];
					*(ctx->ptr + (j * 4) + 3) = V210_black[3];
				}
				line <<= 1;
			}
			ctx->ptr += (ctx->strideBytes / 4);
		}
	}
    
	return 0;
}

static void V210_painter_moveto(struct V210_painter_s *ctx, int x, int y)
{
	ctx->ptr = ctx->frame + ((x * 6) * 4);
	ctx->ptr += ((y * 8 * 4) * (ctx->strideBytes / 4));

	/* X */
#if 0
	ctx->ptr = ctx->frame + ((pixelX / 6) * V210_BLOCK_WIDTH_PER_PIXEL_GROUP);

	/* Y */
	ctx->ptr += (pixelY * (ctx->strideBytes / 4));
#endif
}

static int V210_render_character_at(struct V210_painter_s *ctx, unsigned char letter, int x, int y)
{
        if (letter > 0x9f)
                return -1;

        V210_painter_moveto(ctx, x, y);
        V210_render_character(ctx, letter);

        return 0;
}

void V210_painter_draw_ascii_at(struct V210_painter_s *ctx, int x, int y, const char *str)
{
	for (int i = 0; i < strlen(str); i++)
		V210_render_character_at(ctx, *(str + i), x + i, y);
}

int V210_painter_reset(struct V210_painter_s *ctx, uint8_t *frame, int widthPixels, int heightPixels, int strideBytes, int interlaced)
{
        ctx->frame = (uint32_t *)frame;
        ctx->ptr = ctx->frame;

        ctx->widthPixels  = widthPixels;
        ctx->heightPixels = heightPixels;
        ctx->strideBytes  = strideBytes;
	ctx->interlaced   = interlaced;

        return 0;
}

