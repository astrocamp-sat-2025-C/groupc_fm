#pragma once

// 現在の方位角（degrees）を外部で参照する変数（他ファイルで定義されている想定）
extern float *g_yaw_deg;

// track 関数の宣言
void track(float delta_yaw);