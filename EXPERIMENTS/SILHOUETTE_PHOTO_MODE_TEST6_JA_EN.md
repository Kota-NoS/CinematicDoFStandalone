# Silhouette Photo Mode UI Test 6 / シルエット撮影モード UI Test 6

This is a crash-fix build for Silhouette UI Test 5. The silhouette renderer, compact window, colors, and hotkey behavior are unchanged.

これはSilhouette UI Test 5のCTD修正版です。シルエット描画、小窓、色、ホットキーの挙動は変更していません。

## Fixed / 修正内容

- Test 5 called `igGetItemRectMin()` and `igGetItemRectMax()` with the wrong C ABI signature.
- SKSE Menu Framework writes each result into an `ImVec2*` supplied by the caller. Test 5 incorrectly treated the result as a returned `ImVec2`, causing an invalid write when the Depth of Field page rendered.
- Test 6 supplies valid output storage for both calls.
- No ordinary DoF or silhouette shader code was changed.

- Test 5では`igGetItemRectMin()`と`igGetItemRectMax()`のC ABIシグネチャを誤って呼び出していました。
- SKSE Menu Frameworkは呼び出し側から渡された`ImVec2*`へ結果を書き込みます。Test 5は`ImVec2`を戻り値で受け取る関数として扱ったため、Depth of Field画面の描画時に不正な書込みが発生していました。
- Test 6では両方の関数へ有効な出力領域を渡します。
- 通常DoFおよびシルエット用シェーダーは変更していません。

## Install / 導入

1. Disable and remove Silhouette UI Test 5.
2. Keep the official CinematicDoFStandalone 1.0.1 package enabled.
3. Install this ZIP as a separate MO2 mod and place it after the official package.
4. Disable other Cinematic DoF test DLLs, including moon/water diagnostics.
5. Restart Skyrim before testing; a loaded SKSE DLL cannot be replaced safely while the game is running.

1. Silhouette UI Test 5を無効化または削除してください。
2. 正式版CinematicDoFStandalone 1.0.1は有効のままにしてください。
3. このZIPをMO2の独立Modとして導入し、正式版より後へ配置してください。
4. 月・水診断版を含む他のCinematic DoFテストDLLは無効にしてください。
5. 起動中のSKSE DLLは安全に置換できないため、確認前にSkyrimを再起動してください。

The ZIP intentionally contains no INI. Existing DoF settings, silhouette colors, language, and assigned hotkeys are preserved.

ZIPにはINIを含めていません。現在のDoF設定、シルエット色、言語、登録済みホットキーは保持されます。

## Focused checklist / 確認項目

- Open the Mod Control Panel and select Depth of Field; confirm it no longer crashes.
- Confirm the person-shaped launcher icon appears and opens the compact silhouette window.
- Confirm the icon is white while OFF and orange while ON.
- Confirm the assigned silhouette hotkey still toggles the mode.
- Confirm ordinary DoF and silhouette rendering remain unchanged.

- Mod Control PanelでDepth of Fieldを選択し、CTDしないことを確認する。
- 人物型の起動アイコンが表示され、小窓を開けることを確認する。
- OFF時は白、ON時は橙色になることを確認する。
- 登録済みのシルエット用ホットキーで引き続き切り替えられることを確認する。
- 通常DoFとシルエット描画が変わっていないことを確認する。
