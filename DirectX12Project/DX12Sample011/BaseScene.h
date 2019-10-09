// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __BASESCENE_H__
#define __BASESCENE_H__
// --------------- IncludeFile ---------------
#include "Global.h"
#include "SceneModule.h"
#include "SceneChanger.h"
#include "InputDevice.h"
// ---------------- クラス宣言 ---------------
class CBaseScene : public CSceneModule
{
protected:
	CSceneChanger*	m_pSceneChanger;		// シーンチェンジャーのポインタ
											//(シーンチェンジャーを継承したマネージャーのポインタ)
public:
	CBaseScene(CSceneChanger*);
	virtual ~CBaseScene();
	virtual HRESULT Initialize() override;	// 初期化関数
	virtual void Manage();					// 制御関数
	virtual void Render() override;			// 描画関数
};
#endif /* __BASESCENE_H__ */