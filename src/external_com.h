#pragma once

extern const char* externalInCommandPrefix;
extern const char* externalOutCommandPrefix;

void EC_Init();
void EC_Shutdown();
void EC_ProcessReceivedEvents();
void EC_BroadcastChatMessage(const char* name, const char* message);
void EC_BroadcastNetworkCommand(const char* name, int len, const char* data);
