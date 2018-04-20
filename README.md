# ini
ini file parser

## 支持功能
- [x] Section
- [x] Multi-line: line end with '\\'
- [x] Read
- [ ] Write: next version
- [x] Array: key end with "[]"
- [x] Comments: use '#' or ';'

## Example
```
[my-section]
# array
arr[] = a
arr[]=b
arr[]="c"
arr[]='d'

# is different with arr[]
arr=12345678

# key1=123
key1=123#abc

# key2=123#abc
key2='123#abc'

# multi line, key3=abcdefg
key3=abcd\
efg

```
