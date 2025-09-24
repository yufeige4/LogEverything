
#pragma once

#include "CoreMinimal.h"


#pragma region Log

DECLARE_LOG_CATEGORY_EXTERN(LogEverythingPlugin, Log, All)

#ifndef LE_SYSTEM_LOG
#define LE_SYSTEM_LOG(Format,...)\
{\
UE_LOG(LogEverythingPlugin, Log, TEXT("[LogEverything] " Format), ##__VA_ARGS__)\
}
#endif

#ifndef LE_SYSTEM_WARNING
#define LE_SYSTEM_WARNING(Format,...)\
{\
UE_LOG(LogEverythingPlugin, Warning, TEXT("[LogEverything] " Format), ##__VA_ARGS__)\
}
#endif

#ifndef LE_SYSTEM_ERROR
#define LE_SYSTEM_ERROR(Format,...)\
{\
UE_LOG(LogEverythingPlugin, Error, TEXT("[LogEverything] " Format), ##__VA_ARGS__)\
}
#endif

#pragma endregion