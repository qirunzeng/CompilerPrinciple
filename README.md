# PL0

使用：`cd build; cmake ..; make; ./pl0_compiler`

## TODO

- [] 实现注释功能：
    - [] 块注释：从 `/*` 开始到第一个 `*/` 结束
    - [] 行注释：从 `//` 开始到行尾 `\n`
- [] 增加条件算符：
    - [] `&&`
    - [] `||`
    - [] `!`，不用
    - [] 短路运算：
        - [] 对于 `a || b`，若 `a` 为真则不计算 `b` 的真假
        - [] 对于 `a && b`，若 `a` 为假则不计算 `b` 的真假
    - [] 将 0 视为假，非 0 视为真
- [] 数组
- [] 传参
- [] 语句：
    - [] `else, elif, exit`
    - [] `return`
    - [] `for`

