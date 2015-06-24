# fdatabase
一个php的文件存储扩展

使用方法：

$db = new fdb("/home/work/data/");

$db->setDb("indexcache2");

$db->store("key1","ccceeeeee");

$db->store("key2","ccceeeeee");

$db->store("key3","ccceeeeee");

echo $db->get("key3");

echo $db->delete("key2");

