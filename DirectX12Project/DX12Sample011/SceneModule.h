// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __SCENEMODULE_H__
#define __SCENEMODULE_H__
// --------------- IncludeFile ---------------
#include "Global.h"
#include "XAudio.h"
// ---------------- ÉNÉâÉXêÈåæ ---------------
class CSceneModule
{
public:
	virtual HRESULT Initialize() = 0;
	virtual void Render() = 0;
};
#endif /* __SCENEMODULE_H__ */