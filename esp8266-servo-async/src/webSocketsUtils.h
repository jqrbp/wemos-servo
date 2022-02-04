#ifndef __WEBSOCKETSUTILS_H__
#define __WEBSOCKETSUTILS_H__

void webSocketBroadcastBin(const unsigned char *payload, unsigned int len);
void webSocketBroadcastTXT(const char *payload);
bool webSocketIsConnected(void);
void webSocketInit();
void webSocketLoop();

#endif
