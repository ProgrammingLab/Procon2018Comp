# ビルドに必要なもの
- 最新辺りの Visual Studio 2017
- OpenSiv3D 0.2.4
    - (https://scrapbox.io/Siv3D/Windows_%E7%89%88%E3%81%AE%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB)

# ビジュアライザの使い方
- まだ ビジュアライザ - AI間のインターフェイスを確立させていないため、ビジュアライズ用のクラスを直接扱ってもらう感じになる
- FieldViewクラスをうまく扱えばビジュアライズ機能が使える
    - FieldView::updateをwhile(s3d::System::Update())の中で呼び続ける(Siv3Dの機能です)
    - FieldView::forwardで盤面を1ターン進める

# AIを作る時
- インターフェイスが確立されていないので、手元で適当にやっちゃってください
- インターフェイスが確立されていないので、リモートリポジトリに手を加えに来るのは待ってください