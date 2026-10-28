#ifndef DISPLAY_H
#define DISPLAY_H

void setupDisplay();

void showFace(const unsigned char *face);
void showFaceFor(const unsigned char *face, unsigned long duration);
void showFaceWithMessageFor(
    const unsigned char *face,
    const char *message,
    unsigned long duration);

#endif
