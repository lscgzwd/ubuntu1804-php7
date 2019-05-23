<?php
require("./yimage.php");

$dir = '/home/yjf/wu';
$str = json_encode([
	"$dir/0.jpg",
	"$dir/1.jpg",
]);
$str0 = json_encode(array_values(array_map(function ($x) use ($dir) {
	return "{$dir}/{$x}";
}, array_filter(scandir($dir), function ($x) {
	return $x != '.' && $x != '..';
}))));

// var_dump(extension_loaded('yimage'));
// var_dump(get_extension_funcs('yimage'));

$json = YImageClassify($str, $str0);
$data = json_decode($json);
var_dump($data);
foreach ($data as $item) {
	$name = $item->name;
	$rotation = $item->rotation;
	$image = imagecreatefromjpeg($name);
	$white = imagecolorallocate($image, 255, 255, 255);
	$img = imagerotate($image, $rotation, $white);
	imagejpeg($img, '/home/yjf/cooked-wu/' . basename($name));
	imagedestroy($image);
}

