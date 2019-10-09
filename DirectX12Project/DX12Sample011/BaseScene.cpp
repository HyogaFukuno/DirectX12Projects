// --------------- IncludeFile ---------------
#include "BaseScene.h"

// コンストラクタ
CBaseScene::CBaseScene(CSceneChanger* pSceneChanger)
{
	m_pSceneChanger = pSceneChanger;
}

// デストラクタ
CBaseScene::~CBaseScene()
{

}

// 初期化
HRESULT CBaseScene::Initialize()
{
	return S_OK;
}

// 制御関数
void CBaseScene::Manage()
{

}

// 描画関数
void CBaseScene::Render()
{

}