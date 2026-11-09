# CinematicDoFStandalone 1.0.9

## 日本語

- 深度フレームで、プレイヤー・近くの対象・コンソール対象から、枠の手前に表示する対象を選べるようになりました。人物や動物に加え、読み込み済みの物も選べます。
- 深度フレームを開くと、プレイヤーの周囲30 mの候補を距離順に最大64件表示できるよう準備します。名前のある物は候補に入り、名前のない物はコンソールから選べます。「対象を更新」で再検索します。
- 「対象選択時にピントも追従させる」をオンにすると、枠の対象へ一時的にピントを合わせます。プレイヤーへ戻すか深度フレームをオフにすると、元のフォーカス設定が適用されます。オプションは初期状態でオフです。
- 対象はプレイ中に保持し、セーブをロードするとプレイヤーに戻ります。一部の物やエフェクトは、描画方式によって枠の手前に出ない場合があります。

新規導入には Full を使用してください。設定を引き継ぐ場合は、MO2 で Update を既存のフル版にマージしてください。別途導入した試作版は無効にしてください。

## English

- Choose the player, a nearby subject, or the console selection to bring in front of the Depth Frame. Loaded objects can be selected alongside humanoid actors and creatures.
- Opening Depth Frame scans within 30 m of the player and prepares up to 64 candidates in distance order. Named objects appear in the list, and unnamed objects can be selected through the console. Use Refresh Subjects to scan again.
- Enable Focus Follows Frame Subject to temporarily focus on the selected subject. Returning to the player or disabling Depth Frame resumes your previous focus settings. This option is off by default.
- The selection lasts until you load a save, when it resets to the player. Some objects or effects may render through passes that the frame mask cannot capture.

Use Full for a new installation. To keep your settings, merge Update into the installed full version in MO2. Disable any separate trial overlays.
