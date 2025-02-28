---
{
    "title": "JDBC",
    "language": "zh-CN"
}
---

<!-- 
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
-->


# JDBC

JDBC Catalog 通过标准 JDBC 协议，连接其他数据源。

## 使用限制

1. 支持 MySQL、PostgreSQL、Oracle、Clickhouse

## 创建 Catalog

1. MySQL

	```sql
	CREATE CATALOG jdbc_mysql PROPERTIES (
	    "type"="jdbc",
	    "user"="root",
	    "password"="123456",
	    "jdbc_url" = "jdbc:mysql://127.0.0.1:3306/demo",
	    "driver_url" = "mysql-connector-java-5.1.47.jar",
	    "driver_class" = "com.mysql.jdbc.Driver"
	)
	```
	
2. PostgreSQL

	```sql
	CREATE CATALOG jdbc_postgresql PROPERTIES (
	    "type"="jdbc",
	    "user"="root",
	    "password"="123456",
	    "jdbc_url" = "jdbc:postgresql://127.0.0.1:5449/demo",
	    "driver_url" = "postgresql-42.5.1.jar",
	    "driver_class" = "org.postgresql.Driver"
	);
	```
	
	映射 PostgreSQL 时，Doris 的一个 Database 对应于 PostgreSQL 中指定 Catalog（如示例中 `jdbc_url` 参数中 "demo"）下的一个 Schema。而 Doris 的 Database 下的 Table 则对应于 PostgreSQL 中，Schema 下的 Tables。即映射关系如下：
	
	|Doris | PostgreSQL |
	|---|---|
	| Catalog | Database | 
	| Database | Schema |
	| Table | Tablet |


3. Oracle

	```sql
	CREATE RESOURCE jdbc_oracle PROPERTIES (
		"type"="jdbc",
		"user"="root",
		"password"="123456",
		"jdbc_url" = "jdbc:oracle:thin:@127.0.0.1:1521:helowin",
		"driver_url" = "ojdbc6.jar",
		"driver_class" = "oracle.jdbc.driver.OracleDriver"
	);
	```
	
	映射 Oracle 时，Doris 的一个 Database 对应于 Oracle 中的一个 User（如示例中 `jdbc_url` 参数中 "helowin"）。而 Doris 的 Database 下的 Table 则对应于 Oracle 中，该 User 下的有权限访问的 Table。即映射关系如下：

	|Doris | PostgreSQL |
	|---|---|
	| Catalog | Database | 
	| Database | User |
	| Table | Table |
	
	
4. Clickhouse

	```sql
	CREATE RESOURCE jdbc_clickhouse PROPERTIES (
	    "type"="jdbc",
	    "user"="root",
	    "password"="123456",
	    "jdbc_url" = "jdbc:clickhouse://127.0.0.1:8123/demo",
	    "driver_url" = "clickhouse-jdbc-0.3.2-patch11-all.jar",
	    "driver_class" = "com.clickhouse.jdbc.ClickHouseDriver"
	);
	```
	
### 参数说明

参数 | 是否必须 | 默认值 | 说明 
--- | --- | --- | --- 
`user` | 是 | | 对应数据库的用户名 |
`password` | 是 |   | 对应数据库的密码 |
`jdbc_url ` | 是 |  | JDBC 连接串 |
`driver_url ` | 是 |  | JDBC Driver Jar 包名称* |
`driver_class ` | 是 |  | JDBC Driver Class 名称 |

> `driver_url` 可以通过以下三种方式指定：
> 
> 1. 文件名。如 `mysql-connector-java-5.1.47.jar`。需将 Jar 包预先存放在 FE 和 BE 部署目录的 `jdbc_drivers/` 目录下。系统会自动在这个目录下寻找。该目录的位置，也可以由 fe.conf 和 be.conf 中的 `jdbc_drivers_dir` 配置修改。
> 
> 2. 本地绝对路径。如 `file:///path/to/mysql-connector-java-5.1.47.jar`。需将 Jar 包预先存放在所有 FE/BE 节点指定的路径下。
> 
> 3. Http 地址。如：`https://doris-community-test-1308700295.cos.ap-hongkong.myqcloud.com/jdbc_driver/mysql-connector-java-5.1.47.jar`。系统会从这个 http 地址下载 Driver 文件。仅支持无认证的 http 服务。

## 列类型映射

### MySQL

| MYSQL Type | Doris Type | Comment |
|---|---|---|
| BOOLEAN | BOOLEAN | |
| TINYINT | TINYINT | |
| SMALLINT | SMALLINT | |
| MEDIUMINT | INT | |
| INT | INT | |
| BIGINT | BIGINT | |
| UNSIGNED TINYINT | SMALLINT | Doris没有UNSIGNED数据类型，所以扩大一个数量级|
| UNSIGNED MEDIUMINT | INT | Doris没有UNSIGNED数据类型，所以扩大一个数量级|
| UNSIGNED INT | BIGINT |Doris没有UNSIGNED数据类型，所以扩大一个数量级 |
| UNSIGNED BIGINT | STRING | |
| FLOAT | FLOAT | |
| DOUBLE | DOUBLE | |
| DECIMAL | DECIMAL | |
| DATE | DATE | |
| TIMESTAMP | DATETIME | |
| DATETIME | DATETIME | |
| YEAR | SMALLINT | |
| TIME | STRING | |
| CHAR | CHAR | |
| VARCHAR | STRING | |
| TINYTEXT、TEXT、MEDIUMTEXT、LONGTEXT、TINYBLOB、BLOB、MEDIUMBLOB、LONGBLOB、TINYSTRING、STRING、MEDIUMSTRING、LONGSTRING、BINARY、VARBINARY、JSON、SET、BIT | STRING | |
|Other| UNSUPPORTED |

### PostgreSQL

 POSTGRESQL Type | Doris Type | Comment |
|---|---|---|
| boolean | BOOLEAN | |
| smallint/int2 | SMALLINT | |
| integer/int4 | INT | |
| bigint/int8 | BIGINT | |
| decimal/numeric | DECIMAL | |
| real/float4 | FLOAT | |
| double precision | DOUBLE | |
| smallserial | SMALLINT | |
| serial | INT | |
| bigserial | BIGINT | |
| char | CHAR | |
| varchar/text | STRING | |
| timestamp | DATETIME | |
| date | DATE | |
| time | STRING | |
| interval | STRING | |
| point/line/lseg/box/path/polygon/circle | STRING | |
| cidr/inet/macaddr | STRING | |
| bit/bit(n)/bit varying(n) | STRING | `bit`类型映射为doris的`STRING`类型，读出的数据是`true/false`, 而不是`1/0` |
| uuid/josnb | STRING | |
|Other| UNSUPPORTED |

### Oracle

 ORACLE Type | Doris Type | Comment |
|---|---|---|
| number(p) / number(p,0) |  | Doris会根据p的大小来选择对应的类型：`p < 3` -> `TINYINT`; `p < 5` -> `SMALLINT`; `p < 10` -> `INT`; `p < 19` -> `BIGINT`; `p > 19` -> `LARGEINT` |
| number(p,s) | DECIMAL | |
| decimal | DECIMAL | |
| float/real | DOUBLE | |
| DATE | DATETIME | |
| CHAR/NCHAR | STRING | |
| VARCHAR2/NVARCHAR2 | STRING | |
| LONG/ RAW/ LONG RAW/ INTERVAL | STRING | |
|Other| UNSUPPORTED |

### Clickhouse

| ClickHouse Type        | Doris Type | Comment                                             |
|------------------------|------------|-----------------------------------------------------|
| Bool                   | BOOLEAN    |                                                     |
| String                 | STRING     |                                                     |
| Date/Date32            | DATE       |                                                     |
| DateTime/DateTime64    | DATETIME   | 对于超过了Doris最大的DateTime精度的数据，将截断处理                    |
| Float32                | FLOAT      |                                                     |
| Float64                | DOUBLE     |                                                     |
| Int8                   | TINYINT    |                                                     |
| Int16/UInt8            | SMALLINT   | Doris没有UNSIGNED数据类型，所以扩大一个数量级                       |
| Int32/UInt16           | INT        | Doris没有UNSIGNED数据类型，所以扩大一个数量级                       |
| Int64/Uint32           | BIGINT     | Doris没有UNSIGNED数据类型，所以扩大一个数量级                       |
| Int128/UInt64          | LARGEINT   | Doris没有UNSIGNED数据类型，所以扩大一个数量级                       |
| Int256/UInt128/UInt256 | STRING     | Doris没有这个数量级的数据类型，采用STRING处理                        |
| DECIMAL                | DECIMAL    | 对于超过了Doris最大的Decimal精度的数据，将映射为STRING                |
| Enum/IPv4/IPv6/UUID    | STRING     | 在显示上IPv4,IPv6会额外在数据最前面显示一个`/`,需要自己用`split_part`函数处理 |
|Other| UNSUPPORTED |

## 常见问题

1. 除了 MySQL,Oracle,PostgreSQL,SQLServer,ClickHouse 是否能够支持更多的数据库

   目前Doris只适配了 MySQL,Oracle,PostgreSQL,SQLServer,ClickHouse. 关于其他的数据库的适配工作正在规划之中，原则上来说任何支持JDBC访问的数据库都能通过JDBC外表来访问。如果您有访问其他外表的需求，欢迎修改代码并贡献给Doris。

2. 读写 MySQL外表的emoji表情出现乱码

    Doris进行jdbc外表连接时，由于mysql之中默认的utf8编码为utf8mb3，无法表示需要4字节编码的emoji表情。这里需要在建立mysql外表时设置对应列的编码为utf8mb4,设置服务器编码为utf8mb4,JDBC Url中的characterEncoding不配置.（该属性不支持utf8mb4,配置了非utf8mb4将导致无法写入表情，因此要留空，不配置）

	可全局修改配置项
	
	```
	修改mysql目录下的my.ini文件（linux系统为etc目录下的my.cnf文件）
	[client]
	default-character-set=utf8mb4
	
	[mysql]
	设置mysql默认字符集
	default-character-set=utf8mb4
	
	[mysqld]
	设置mysql字符集服务器
	character-set-server=utf8mb4
	collation-server=utf8mb4_unicode_ci
	init_connect='SET NAMES utf8mb4
	
	修改对应表与列的类型
	ALTER TABLE table_name MODIFY  colum_name  VARCHAR(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
	ALTER TABLE table_name CHARSET=utf8mb4;
	SET NAMES utf8mb4
	
	```

3. 读 MySQL 外表时，DateTime="0000:00:00 00:00:00"异常报错: "CAUSED BY: DataReadException: Zero date value prohibited"

   这是因为JDBC中对于该非法的DateTime默认处理为抛出异常，可以通过参数 `zeroDateTimeBehavior`控制该行为。
   
   可选参数为: `EXCEPTION`,`CONVERT_TO_NULL`,`ROUND`, 分别为：异常报错，转为NULL值，转为 "0001-01-01 00:00:00";
   
   可在url中添加: `"jdbc_url"="jdbc:mysql://IP:PORT/doris_test?zeroDateTimeBehavior=convertToNull"`

4. 读取 MySQL 外表或其他外表时，出现加载类失败
   
   如以下异常：
   
   	```
   failed to load driver class com.mysql.jdbc.driver in either of hikariconfig class loader
   ```
  
   这是因为在创建resource时，填写的driver_class不正确，需要正确填写，如上方例子为大小写问题，应填写为 `"driver_class" = "com.mysql.jdbc.Driver"`

5. 读取 MySQL 问题出现通信链路异常

   如果出现如下报错：

   ```
	ERROR 1105 (HY000): errCode = 2, detailMessage = PoolInitializationException: Failed to initialize pool: Communications link failure
    
	The last packet successfully received from the server was 7 milliseconds ago.  The last packet sent successfully to the server was 4 milliseconds ago.
	CAUSED BY: CommunicationsException: Communications link failure
	    
	The last packet successfully received from the server was 7 milliseconds ago.  The last packet sent successfully to the server was 4 milliseconds ago.
	CAUSED BY: SSLHandshakeExcepti
   ```
   
   可查看be的be.out日志
   
   如果包含以下信息：
   
   ```
   WARN: Establishing SSL connection without server's identity verification is not recommended. 
   According to MySQL 5.5.45+, 5.6.26+ and 5.7.6+ requirements SSL connection must be established by default if explicit option isn't set. 
   For compliance with existing applications not using SSL the verifyServerCertificate property is set to 'false'. 
   You need either to explicitly disable SSL by setting useSSL=false, or set useSSL=true and provide truststore for server certificate verification.
   ```

   可在创建 Catalog 的 `jdbc_url` 把JDBC连接串最后增加 `?useSSL=false` ,如 `"jdbc_url" = "jdbc:mysql://127.0.0.1:3306/test?useSSL=false"`
