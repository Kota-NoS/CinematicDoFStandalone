# CinematicDoFStandalone 1.0.0

## 日本語

1.0.0はCinematic DoF Standaloneの最初の正式安定版です。0.8.32で完成した描画、UI、9プリセット、会話フォーカス、FSR有効領域補正、HDR/PBR安全化、絞り形状ボケを維持し、Skyrim 1.7.104対応と限定的な低設定人物保護を統合しています。

### 主な変更

- Skyrim AE 1.7.104とAddress Library v5形式へ正式対応しました。従来のAddress Library v1/v2読込経路も維持するため、Skyrim AE 1.6.1170との互換性を保ちます。
- 絞り形状ボケを正式機能へ昇格しました。羽根枚数、丸み、形状の強さ、回転、明るいボケの強調、仕上げの滑らかさを調整できます。描画処理は実機確認済みの0.8.32基準から変更していません。
- Community Shadersを使わない環境では、SSRまたは64-bit HDRが無効な場合だけ、対象追従中の人物保護を有効にします。SAO無効だけでは発動しません。
- 人物保護の頭部領域は中心と外径を変えず、半径の内側82%を完全保護し、残り18%を滑らかな遷移にします。
- Community Shadersなし、`bUse64bitsHDRRenderTarget=0`、有効な追従対象あり、という条件が揃ったフレームだけ、手前ピント範囲の実効値を`max(保存値, 0.15 m)`にします。INI、プリセット、UIに保存された値は変更しません。
- `bUse64bitsHDRRenderTarget=1`かつSSR有効の通常環境では、スタンドアロン人物保護と0.15 m補助は両方とも無効です。
- Community Shaders使用時の既存深度フォールバックは変更していません。SAO、SSR、64-bit HDRのいずれかが無効な場合に従来どおり判定します。
- INI形式、プリセットID、保存値の互換性は変更していません。0.8.32以前からINIなし更新版をMergeすれば、ユーザー設定を維持できます。

### 実機確認

- Skyrim AE 1.6.1170と1.7.104で起動、DoF描画、UI、対象追従、ピント固定、画面位置AF、ホットキーを確認しました。
- 1.7.104でAddress Library v5データベースから必要な再配置IDを解決し、実際のDoF描画まで到達することを確認しました。
- Community Shadersなしの1.6.1170で、`bUse64bitsHDRRenderTarget=0`だけを変更すると人物ボケが再現することを確認しました。
- 最終保護を有効にすると人物が鮮明になり、通常距離ではHDR有効時との違いが判別困難な範囲に収まることを確認しました。
- 極端な近影では0.15 m補助による小さな差がありますが、周辺の正常なDoF表現を保ったまま人物ボケを防ぐ結果を確認しました。
- HDR、SSR、SAOが有効でCommunity Shadersを使わない通常構成では、ログ上で両保護が無効になることを確認しました。

### 既知の制限

非常に大きなボケを低い「ぼかし品質」で描画すると、サンプル密度不足により明るいボケが輪や点へ分裂して見える場合があります。必要に応じて品質を上げるか、手前／奥最大ぼかしを弱めてください。

## English

Version 1.0.0 is the first stable release of Cinematic DoF Standalone. It preserves the completed 0.8.32 rendering, UI, nine presets, dialogue focus, FSR active-area correction, HDR/PBR safety, and aperture-shaped bokeh while adding official Skyrim 1.7.104 support and narrowly scoped low-setting subject protection.

### Main changes

- Official support for Skyrim AE 1.7.104 and the Address Library v5 database format. The legacy v1/v2 reader remains available, preserving Skyrim AE 1.6.1170 compatibility.
- Aperture-shaped bokeh is now a completed feature. Blade count, roundness, shape strength, rotation, Highlight Boost, and post-blur smoothing remain adjustable. Its rendering algorithm is unchanged from the in-game-validated 0.8.32 baseline.
- Without Community Shaders, tracked-subject protection activates only when SSR or 64-bit HDR is disabled. Disabled SAO alone does not activate it.
- The head guard keeps the same centre and outer radius, fully protects the inner 82% of its radius, and uses the remaining 18% as a smooth transition.
- Only frames with no Community Shaders, explicit `bUse64bitsHDRRenderTarget=0`, and a valid tracked target use an effective near-focus range of `max(saved value, 0.15 m)`. The saved INI, preset, and UI value are unchanged.
- In a normal standalone configuration with `bUse64bitsHDRRenderTarget=1` and SSR enabled, both the standalone target guard and 0.15 m assist are disabled.
- The established Community Shaders depth fallback is unchanged and continues to consider disabled SAO, SSR, or 64-bit HDR.
- The INI schema, preset IDs, and saved-value compatibility are unchanged. Use the INI-free Update-Merge package to preserve an existing configuration.

### In-game verification

- Launch, DoF rendering, UI, Target Tracking, Fixed Focus, Screen AF, and the hotkey were checked on Skyrim AE 1.6.1170 and 1.7.104.
- Skyrim 1.7.104 resolved the required relocation IDs from an Address Library v5 database and reached live DoF rendering.
- With Community Shaders absent on 1.6.1170, changing only `bUse64bitsHDRRenderTarget` to `0` reproduced the tracked-character blur.
- The final protection kept the subject sharp while ordinary-distance comparisons remained visually equivalent to the HDR-enabled path.
- Extreme close foreground comparisons showed a small difference from the 0.15 m assist, accepted because it prevents subject blur without removing the surrounding DoF effect.
- With HDR, SSR, and SAO enabled and no Community Shaders, the runtime log confirmed that both protection paths were disabled.

### Known limitation

At low `Blur Quality`, very large blur discs can split into visible rings or points because the gather has insufficient sample density. Raise quality or reduce the near/far maximum blur when necessary.
