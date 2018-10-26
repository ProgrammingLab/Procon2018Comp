# 第29回高専プロコン競技部門
- 登録番号: 30057
- チーム名: 計算じゃ解けない疑問は手と手で探るの
- 作品タイトル: 機械学習ではない
- 学校名: 久留米工業高等専門学校

# リポジトリ概要
機械学習（AlphaGo Zeroを参考にした強化学習）のソースコードが一応残っているが、本番までにうまくいかないと判断したため現在使っていない。

## 使用ライブラリ等
- 機械学習: Python + TensorFlow, C++ + boost
- UI: C++ + OpenSiv3D
- AI: C++
- C++のコードは全てVisual Studioにより管理している

## バイナリファイル
- https://github.com/ProgrammingLab/Procon2018Comp/releases/tag/1.0
    - AIと対戦できる

## 主なディレクトリ構成
- `./Procon2018/` UIやAIを含む中心的なプロジェクト。Windowsでの実行を想定
- `./Procon2018/FieldView.h, FieldView.cpp, Playground.h, Playground.cpp` **UIのコード**  
- `./Procon2018/Shared/` OpenSiv3Dを使用せず、他のプロジェクトからも再利用できるようにしたコード群  
- `./Procon2018/Shared/Field.h, Field.cpp` **ゲーム進行のための基本機能が実装されたクラス、その他ヘルパークラス**  
- `./Procon2018/WinjAI/WinjAI.h` **AIのコード**  
- `./Python/` 機械学習時に使用したコード群  
- `./Python/library.py` モデルの定義やその他諸々を含むライブラリ的ファイル  
- `./Python/learner.py` モデルの学習に使用したプログラム  
- `./SelfPlay_win/` 強化学習のための自己対局の実装  
- `./Procon2018.sln` Visual Studio ソリューションファイル  

# ビルドに必要なもの
- 最新辺りの Visual Studio 2017
- OpenSiv3D 0.2.4
    - (https://scrapbox.io/Siv3D/Windows_%E7%89%88%E3%81%AE%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB)
