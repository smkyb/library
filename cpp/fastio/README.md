# 高速入出力
fread(_unlocked)などを使用したものです。  
flushはできません。  
<br>
## 使い方
・ fread_unlocked、fwrite_unlockedが使えない場合、  
　 FIstream、FOstreamの最上部にある 「#ifndef LOCAL」から始まる3行を消してください
