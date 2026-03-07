#ifndef USBHIDHOST_H
#define USBHIDHOST_H



void OnPress(int key)
{
	Serial.print("key '");
	Serial.print((char)key);
	Serial.print("'  ");
	Serial.println(key);
	//Serial.print("key ");
	//Serial.print((char)keyboard1.getKey());
	//Serial.print("  ");
	//Serial.print((char)keyboard2.getKey());
	//Serial.println();
}

void OnRawPress(uint8_t keycode)
{
	Serial.print("raw key press: ");
	Serial.println((int)keycode);
}

void OnRawRelease(uint8_t keycode)
{
	Serial.print("raw key release: ");
	Serial.println((int)keycode);
}

#endif