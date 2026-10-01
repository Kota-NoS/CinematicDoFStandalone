# Silhouette Photo Mode UI Test 5 / シルエット撮影モード UI Test 5

This independent test build is based on CinematicDoFStandalone 1.0.1. It keeps the verified Test 3 rendering path and Test 4 hotkey/window behavior. Only the launcher icon is changed.

これはCinematicDoFStandalone 1.0.1を基準にした独立テスト版です。動作確認済みのTest 3描画経路とTest 4のホットキー・小窓の挙動を維持し、起動アイコンだけを変更しています。

## Changes from Test 4 / Test 4からの変更

- The `■` character was not included in the active Menu Framework font and appeared as `?`.
- The launcher now draws a head-and-shoulders silhouette directly with ImGui primitives, without relying on a font glyph or an external image file.
- The person icon is white while silhouette mode is OFF and orange while it is ON.
- If the required drawing exports are unavailable on an older Menu Framework build, the launcher falls back to an ASCII `S` instead of becoming blank.
- Rendering, colors, the compact window, and both hotkeys are otherwise unchanged.

- `■`が使用中のMenu Frameworkフォントに含まれておらず、`?`として表示されていました。
- 起動ボタンはフォント文字や外部画像へ依存せず、ImGuiの基本図形で頭と肩の人物シルエットを直接描きます。
- シルエットOFF時は白、ON時は橙色の人物アイコンになります。
- 古いMenu Frameworkで必要な描画関数が利用できない場合も、空欄や`?`ではなくASCIIの`S`へフォールバックします。
- 描画処理、2色、小窓、DoF／シルエットの各ホットキーは変更していません。

## Install / 導入

1. Install the official CinematicDoFStandalone 1.0.1 Full or Update package first.
2. Disable Silhouette Test 3/4, moon/water diagnostic builds, and any other Cinematic DoF test DLL.
3. Install this ZIP as a separate MO2 mod and place it after the official package.
4. Choose `Merge` when MO2 asks. Do not choose `Replace`.

1. 先に正式版CinematicDoFStandalone 1.0.1のFull版またはUpdate版を導入してください。
2. Silhouette Test 3/4、月・水診断版など、他のCinematic DoFテストDLLを無効化してください。
3. このZIPをMO2の独立Modとして導入し、正式版より後へ配置してください。
4. MO2で確認された場合は「1つにまとめる（Merge）」を選び、「置き換える（Replace）」は選ばないでください。

The ZIP intentionally contains no INI, so the current DoF, silhouette colors, language, and assigned hotkeys are preserved.

このZIPにはINIを含めていないため、現在のDoF設定・シルエット色・言語・登録済みホットキーは保持されます。

## Focused checklist / 確認項目

- Confirm a person-shaped icon appears beside the DoF hotkey instead of `?`.
- Click it repeatedly and confirm the compact silhouette window opens each time.
- Confirm the icon changes from white to orange when silhouette mode becomes active, and returns to white when disabled.
- Close all menus and confirm the assigned silhouette hotkey still toggles the effect.
- Confirm ordinary DoF and the verified silhouette rendering result are unchanged.

- DoFホットキー横に`?`ではなく人物型アイコンが表示されることを確認する。
- 何度かクリックし、毎回シルエット設定の小窓が開くことを確認する。
- シルエットONでアイコンが白から橙色へ変わり、OFFで白へ戻ることを確認する。
- すべてのメニューを閉じ、登録済みシルエットホットキーで引き続き切り替えられることを確認する。
- 通常DoFと確認済みのシルエット描画結果が変わっていないことを確認する。
