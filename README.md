# JsonCpp
使用C++实现的一个`JSON`解析器，提供C++实体类与JSON字符串之间的序列化与反序列化操作。*仅为学习之作*。


## LICENSE
源代码在`MIT`协议下发布。具体内容要求见`LICENSE`文件。


## 使用方式
解析器对外提供三个头文件：`JSON.hpp`、`JSONConvert.hpp`、`JSONQuery.hpp`，依次代表了三种递进功能：基础JSON解析与序列化、实体类反序列化操作、JsonPath查询支持。根据需要使用的功能级别，包含对应的头文件即可。

> 反序列化操作已经完整支持，使用方式可以参考测试代码。序列化目前未提供直接扩展，只在内部实现，可以间接使用。

### JSON解析
方法：`parse()`。传入utf-8编码字符串，以及可选的错误码存储地址即可。当解析成功时，会返回一个指向JSON对象类的智能指针对象；如果解析失败，智能指针对象为默认初始化状态。

### 实体类操作
首先使用宏`DESERIALIZE_CLASS`和`DESERIALIZE`将要进行反序列化操作的实体类进行配置。以下是一个示例：

``` cpp
struct people {
    int age;
    std::string name;
}

DESERIALIZE_CLASS(people, DESERIALIZE(age), DESERIALIZE(name));
```

经过上述配置后，就可以直接使用类似如下代码进行反序列化了：

``` cpp
auto token = parse(json_str, nullptr);
if (token) {
    people p = { 0, "" };
    deserialize(p, *token);
}
```

### JsonPath查询
当JSON解析完成后，可以对返回的`json_token`对象执行查询操作。

JsonPath查询语法在下面给出。而使用方式则很简单：

使用`select_token`对返回单个结果的JsonPath查询使用，而需要返回列表时，使用`select_tokens`。

``` cpp
auto token = parse(json_str, nullptr);
if (token) {
    auto result_list = select_tokens(*token, "$..*");
}
```

## JsonPath
类似`XPath`（`XmlPath`）语法，对于`Json`数据对象，也同样有一套路径访问语法，称为`JsonPath`表达式，也称为`JPath`。具体详细语法规则可以参见[JsonPath语法](http://goessner.net/articles/JsonPath/)。下面给出几个示例：

``` JavaScript
// 点标记语法
$.store.book[0].title
// 括号标记语法
$['store']['book'][0]['title']
// 使用索引
$.store.book[(@.length-1)].title
// 过滤器
$.store.book[?(@.price < 10)].title
```

其它更多详细规则，可以从上面链接网址中查看，同时还可以查看与`XPath`语法的类比。

### 支持语法
> 当前JSONPath语法规则按照<https://github.com/json-path/JsonPath>进行实现。尚未完成全部支持。JSONPath语法可在<http://jsonpath.herokuapp.com/>上面进行在线测试。

解析器同时支持点标记语法和括号标记语法。已支持除过滤器和表达式之外的所有语法，可以正常使用，只是在以下几种细节上需要注意：

* 点标记语法和括号标记语法不可混用。
* 当解析器发现表达式中存在`$..*`这种语法时，将整个表达式视为点标记语法。
* 当解析器发现表达式中存在类似`$[*]`或`$..[*]`或`$(array subscription expr)+[*]`的语法时，将整个表达式视为括号标记语法。

另外几点是标准语法规定（或衍生规定），需要注意的细节：

* JsonPath查询字符串中只能使用单引号引用对象的Key名称。
* 数组下标语法中，使用下标或下标列表方式，所有下标值都必须为非负数；而使用slice语法（`[start:end:step]`）时，start和end可以为负值，表示从数组最后一个元素处开始计数。


## 历程
* `v0.1.0`：基本实现从JSON字符串到JSON对象的解析。
* `v0.2.0-beta`：略。
* `v0.3.0`：~~基本完成对普通JsonPath语法的支持。~~（已废弃，JsonPath引擎已经全部进行了重写）
* `v0.4.0`:重构解析过程，调整使用接口，增加实体类反序列化操作。修正字符串和数字解析过程中的错误。重新实现JsonPath查询引擎（支持大部分查询语法）。


## 代码说明
解析器只依赖标准库，基于**C++11**，跨平台。解析方法保证异常安全，方法内部需要访问字符串，由调用者保证解析期间字符串不会被修改。其它接口同理，在调用期间，由调用者维护入参完整性状态。

### 解析方法
解析过程中采用快进式的只读解析方式，基本只需要对字符串扫描一遍即可。

关于`JSON`格式，可以参考<http://www.json.org/index.html>中的定义，其中包含了每种类型的语法定义。

针对每种JSON类型，在代码中分别定义了一种与之相对的类，用于存储其类型值；而为了通用性，在这些实际类型之上，定义了一个名为`json_token`的抽象基类，该类只为提供类型抽象，并方便在容器中包装其它实际的实现类。

当开始解析后，使用类似递归的方式进行解析，因为JSON类型本身就是可以嵌套的。根据扫描遇到的字符，决定采用的解析方式：例如遇到一个`{`符号，说明之后是一段JSON对象类型，所以可以进入对象解析方法进行解析；同理如果遇到`[`符号，执行JSON数组解析方法，遇到`"`符号，开始解析JSON字符串等等。

每一种JSON类型都有其规定的格式，所以在每种特定的类型解析实现中，也会验证JSON格式。以JSON对象为例，它是由一对大括号包围的Key-Value结构类型，其中Key又是JSON字符串，而Value可以是任意的JSON类型，所以JSON对象的解析方法会严格按照这种定义格式进行解析并验证。而递归的存在就是因为**Value值可以是任意的JSON类型**，所以当解析Value时，会递归调用解析方法。

### 字符串&数字解析
字符串和数字可以视为一种**终结类型**，也就是它们的解析不会存在递归调用了。

这里针对JSON字符串和数字，实现了单独的解析方法。因为JSON字符串和数字的格式也有其特定的要求。在解析字符串的时候，主要重心在转义序列的解析，按照标准格式的规定，将转义字符还原为实际字符，同时会将`\uxxxx`这种格式的Unicode字符进行解析并转换为utf-8编码存储；但是在将普通字符串格式化为JSON字符串输出的时候，当前并没有将所有的**非ASCII**编码字符都转义输出，而是维持了原始字符。**所以，当一段JSON字符串中如果包含Unicode转义字符，解析后会保存为普通的utf-8编码字符；然后再将解析后的普通字符串格式化为JSON字符串的时候，之前的非ASCII编码Unicode字符并不会重新编码为`\uxxxx`的形式，而是保持utf-8编码不变。**

数字的解析相比较字符串而言要更麻烦一些，不仅仅因为数字的语法定义规则更复杂多变，同时也因为数字可能存在溢出情况。在内部数据结构中，对于数字采用`int64`和`double`进行存储，虽然基本可以满足大部分情况，但是针对可能存在的溢出情况，仍然需要进行考虑。对于整数的溢出检查，只要在每次更新解析值之前，检查当前解析值是否大于`std::numeric_limits<int64_t>::max() / 10 - 1`即可，如果大于，则说明将当前值“附加到”解析值后面之后，新的解析值就可能超出64位整数的表示范围了，也就是发生了溢出。而对于浮点数，则需要考虑上溢和下溢两种情况，这两种溢出主要体现在指数值上，上溢时指数值过大，下溢则是指数值为负且过小，而在C++中，刚好有关于这两个指数临界值的定义，分别为`std::numeric_limits<double>::max_exponent10`和`std::numeric_limits<double>::min_exponent10`，这两个值就是以10为底数时候的最大和最小指数值了，当检查到实际解析的指数值超出这两个范围，则证明发生了浮点数溢出；而除了这两种情况，浮点数还有一种特殊情况，即不存在指数项但小数点后面的位数过长，此时解析器并不会将其视为错误，而是简单的丢弃超过有效值的部分内容。

### JsonPath引擎
[此部分具体内容后续补充]
