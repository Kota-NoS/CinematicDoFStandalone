# Silhouette Photo Mode UI Test 4 / シルエット撮影モード UI Test 4

This independent test build is based on CinematicDoFStandalone 1.0.1. It keeps the verified Test 3 rendering path and changes only the silhouette controls and hotkey support. It does not connect to or require Screenshot Lighting Assistant.

これはCinematicDoFStandalone 1.0.1を基準にした独立テスト版です。動作確認済みのTest 3描画経路を維持し、シルエット用UIとホットキーだけを変更しています。Screenshot Lighting Assistantとの連携や依存はありません。

## Changes from Test 3 / Test 3からの変更

- The optional controls no longer occupy the main DoF page.
- A white square button beside the DoF hotkey opens a compact independent window.
- The compact window contains the silhouette switch, two colors, and a dedicated keyboard hotkey.
- The silhouette hotkey works while the compact window is closed and does not automatically save the enabled state.
- Assigning the same keyboard key to DoF and silhouette clears the other assignment so one key cannot toggle both modes.

- おまけ機能の設定を通常DoF画面から取り除きました。
- DoFホットキー横の白い四角ボタンから、独立した小窓を開けます。
- 小窓にシルエットON/OFF、2色、専用キーボードホットキーをまとめました。
- シルエット用ホットキーは小窓を閉じていても動作し、切替後のON/OFF状態は自動保存しません。
- DoFとシルエットへ同じキーを登録した場合は、もう一方の登録を解除し、1キーで両方が切り替わる状態を防ぎます。

## Install / 導入

1. Install the official CinematicDoFStandalone 1.0.1 Full or Update package first.
2. Disable or remove Silhouette Test 3, Water Mask tests, and other Cinematic DoF diagnostic DLLs.
3. Install this ZIP as a separate MO2 mod and place it after the official package.
4. Choose `Merge` when MO2 asks. Do not choose `Replace`.

1. 先に正式版CinematicDoFStandalone 1.0.1のFull版またはUpdate版を導入してください。
2. Silhouette Test 3、Water Mask Test版など、他のCinematic DoF診断DLLを無効化または削除してください。
3. このZIPをMO2の独立Modとして導入し、正式版より後へ配置してください。
4. MO2で確認された場合は「1つにまとめる（Merge）」を選び、「置き換える（Replace）」は選ばないでください。

The ZIP intentionally does not contain `CinematicDoFStandalone.ini`, so current DoF, color, and language settings are preserved. `ToggleSilhouette` is added to the existing INI automatically when a silhouette hotkey is assigned.

このZIPには意図的に`CinematicDoFStandalone.ini`を含めていません。現在のDoF・色・言語設定は保持されます。シルエット用ホットキーを登録すると、既存INIへ`ToggleSilhouette`が自動追加されます。

## Use / 使い方

1. Open SKSE Menu Framework and select `Cinematic DoF Standalone` > `Depth of Field`.
2. Press the white square `■` beside the DoF hotkey to open `Silhouette Photo Mode`.
3. Enable the mode, choose both colors, and optionally press its hotkey button before pressing the desired keyboard key.
4. Press `Esc` to cancel hotkey capture, or `Backspace` / `Delete` to clear the hotkey.

1. SKSE Menu Frameworkで`Cinematic DoF Standalone` > `Depth of Field`を開きます。
2. DoFホットキー横の白い`■`を押し、「シルエット撮影モード」の小窓を開きます。
3. モードON/OFFと2色を設定し、必要ならホットキーボタンを押してから登録したいキーボードのキーを押します。
4. ホットキー登録は`Esc`で中止、`Backspace`または`Delete`で解除できます。

Changes apply immediately. Use the compact window's `Save Startup Settings / 次回起動設定を保存` only when you intentionally want to preserve the switch and colors for the next launch. Hotkey assignments themselves are saved immediately.

変更は即時反映されます。ON/OFFと2色を次回起動にも残したい場合だけ、小窓の「次回起動設定を保存」を押してください。ホットキーの登録内容だけは変更時に自動保存されます。

## Focused Test 4 checklist / Test 4確認項目

- Open and close the compact window repeatedly.
- Confirm the main DoF page contains only the white square launcher, not the full silhouette controls.
- Assign a silhouette hotkey, close every menu, and toggle silhouette ON/OFF with it.
- Confirm the hotkey still works after save/load, cell transition, and fast travel.
- Confirm changing the silhouette hotkey does not change the DoF hotkey, except when intentionally assigning the same key.
- Confirm silhouette OFF immediately restores the previous normal DoF result.
- Confirm the Test 3 behavior remains stable in menus, loading screens, maps, and screenshots.

- 小窓を何度か開閉する。
- 通常DoF画面には白い四角の起動ボタンだけがあり、シルエット設定一式が表示されないことを確認する。
- シルエット用ホットキーを登録し、すべてのメニューを閉じてからON/OFFできることを確認する。
- セーブ・ロード、セル移動、ファストトラベル後もホットキーが動くことを確認する。
- 同じキーを意図的に指定した場合を除き、シルエット用ホットキーの変更でDoF用ホットキーが変わらないことを確認する。
- シルエットOFF直後に、以前の通常DoF表示へ戻ることを確認する。
- メニュー、ロード画面、マップ、各撮影方式でTest 3と同じ安定性が維持されていることを確認する。

The rendering limitations are unchanged from Test 3: classification uses only the current main depth buffer and does not identify object types. Water, moons, particles, transparent hair, and alpha vegetation follow whether they write depth.

描画上の制限はTest 3と同じです。現在のメイン深度だけで分類し、オブジェクト種別は判定しません。水面・月・パーティクル・半透明髪・アルファ草木は、深度を書くかどうかで分類されます。
