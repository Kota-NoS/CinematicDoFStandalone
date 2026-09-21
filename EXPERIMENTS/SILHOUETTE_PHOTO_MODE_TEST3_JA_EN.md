# Silhouette Photo Mode Test 3 / シルエット撮影モード Test 3

This is an independent test build based on CinematicDoFStandalone 1.0.1. It does not connect to or require Screenshot Lighting Assistant.

これはCinematicDoFStandalone 1.0.1を基準にした独立テスト版です。Screenshot Lighting Assistantとの連携や依存はありません。

## Install / 導入

1. Install the official CinematicDoFStandalone 1.0.1 Full or Update package first.
2. Install this Test package as a separate MO2 mod and place it after the official package.
3. Disable Water Mask Test builds and other Cinematic DoF diagnostic DLLs.
4. Choose `Merge` when MO2 asks. Do not choose `Replace`.

1. 先に正式版CinematicDoFStandalone 1.0.1のFull版またはUpdate版を導入してください。
2. このTest版をMO2の独立Modとして導入し、正式版より後へ配置してください。
3. Water Mask Test版など、他のCinematic DoF診断DLLは無効にしてください。
4. MO2で確認された場合は「1つにまとめる（Merge）」を選び、「置き換える（Replace）」は選ばないでください。

The ZIP intentionally does not contain `CinematicDoFStandalone.ini`, so the current DoF settings are preserved. If the old INI has no `[Silhouette]` section, silhouette mode starts OFF with safe default colors.

このZIPには意図的に`CinematicDoFStandalone.ini`を含めていないため、現在のDoF設定は保持されます。旧INIに`[Silhouette]`がない場合も、シルエットはOFF・既定色で安全に起動します。

## Use / 使い方

Open SKSE Menu Framework, select `Cinematic DoF Standalone` > `Depth of Field`, then use the separate `Silhouette Photo Mode / シルエット撮影モード` section.

- `Enable Silhouette Mode / シルエット撮影モードを有効にする`
- `Silhouette Color / シルエット色`
- `Background Color / 背景色`

Changes apply immediately. Use `Save Startup Settings / 次回起動設定を保存` only when you want to preserve the mode and colors for the next launch. Silhouette values are independent of all nine DoF presets.

変更は即時反映されます。次回起動にもモードと色を残す場合だけ「次回起動設定を保存」を押してください。シルエット設定は9個のDoFプリセットから完全に独立しています。

## Design / 仕様

- Silhouette mode works even when the normal DoF master switch is OFF.
- While silhouette is ON, dialogue/player/console target tracking, focus updates, dialogue-only prewarm, CoC calculation, and the normal 15 DoF passes are bypassed.
- Turning silhouette OFF immediately returns to the previously active DoF settings.
- Silhouette always stops in the main menu, loading screens, and world/local maps, independently of the normal DoF menu option.
- A silhouette shader/resource failure disables only silhouette mode. Normal DoF remains available; details are written to `CinematicDoFStandalone.log`.

- 通常DoFの主スイッチがOFFでもシルエットだけ使用できます。
- シルエット中は、会話・プレイヤー・コンソール対象追跡、フォーカス更新、会話専用プリウォーム、CoC計算、通常DoFの15パスを実行しません。
- シルエットをOFFにすると、直前まで使用していたDoF設定へ即座に戻ります。
- 通常DoFのメニュー設定に関係なく、タイトル・ロード・ワールドマップ・ローカルマップでは常に停止します。
- シルエット用シェーダー／リソースだけが失敗した場合はシルエットだけを停止します。通常DoFは引き続き利用でき、原因は`CinematicDoFStandalone.log`へ記録されます。

## Known limitations / 既知の制限

This first test uses only the current main depth buffer: clear depth becomes the background color and written depth becomes the silhouette color. It does not identify object types.

初版は現在のメイン深度だけを使用し、クリア深度を背景色、深度が書かれた領域をシルエット色にします。オブジェクト種別の判別は行いません。

- Moons that write depth can become silhouette color.
- Water, particles, fog, transparent hair, and alpha vegetation can fall on either side depending on whether they write depth.
- UI is drawn later and may remain visible.
- Selected colors can differ slightly in screenshots because Skyrim post-processing/exposure continues after this hook.

- 深度を書く月はシルエット色になる場合があります。
- 水面・粒子・霧・半透明髪・アルファ草木は、深度を書くかどうかで分類が変わります。
- UIは後段描画のため、そのまま表示される場合があります。
- このフック後にもSkyrim側のポスト処理や露出が続くため、選択色とスクリーンショットの色が少し異なる場合があります。

## Test checklist / 確認項目

- Silhouette OFF: compare ordinary DoF against official 1.0.1.
- Normal DoF OFF: confirm silhouette still works.
- Toggle silhouette repeatedly and confirm no old frame or solid screen remains.
- Change both colors and confirm immediate updates.
- Confirm dialogue-only DoF, nine presets, Keep Sky Sharp, and target tracking still work after turning silhouette OFF.
- Open/close the main menu, loading screen, world map, and local map.
- Test after load, cell transition, fast travel, and resolution change.
- Test Community Shaders on/off where possible, plus the usual upscaler, HDR, and frame-generation configurations.
- Test Skyrim AE 1.6.1170 and 1.7.104 where available.
- Confirm screenshots from Steam, ENB, and Community Shaders capture the result.

- シルエットOFF時の通常DoFを正式版1.0.1と比較。
- DoF本体OFFでもシルエットが動くことを確認。
- ON/OFFを繰り返し、前フレームや単色画面が残らないことを確認。
- 2色を変更し、即時反映を確認。
- シルエットOFF後、会話専用DoF、9プリセット、Keep Sky Sharp、対象追跡が正常であることを確認。
- タイトル、ロード画面、ワールドマップ、ローカルマップの開閉を確認。
- ロード、セル移動、ファストトラベル、解像度変更後を確認。
- 可能な範囲でCommunity Shadersあり／なし、普段のアップスケーラー・HDR・フレーム生成構成を確認。
- 可能な範囲でSkyrim AE 1.6.1170と1.7.104を確認。
- Steam・ENB・Community Shaders等、普段の撮影方式へ反映されることを確認。

If the screen becomes white, flashes, freezes on an old image, or the effect appears offset, turn silhouette OFF and provide `Documents\My Games\Skyrim Special Edition\SKSE\CinematicDoFStandalone.log` with the test conditions.

白画面、点滅、前フレームの張り付き、座標ずれが出た場合はシルエットをOFFにし、テスト条件と`Documents\My Games\Skyrim Special Edition\SKSE\CinematicDoFStandalone.log`をお知らせください。
