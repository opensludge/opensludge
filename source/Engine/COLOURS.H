//unsigned short int makeColour (byte r, byte g, byte b);

inline unsigned short redValue (unsigned short c) { return (c >> 11) << 3; }
inline unsigned short greenValue (unsigned short c) { return ((c >> 5) & 63) << 2; }
inline unsigned short blueValue (unsigned short c) { return (c & 31) << 3; }

inline unsigned short makeGrey (unsigned short int r) {
	return ((r >> 3) << 11) | ((r >> 2) << 5) | (r >> 3);
}

inline unsigned short makeColour (unsigned short int r, unsigned short int g, unsigned short int b) {
	return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}
