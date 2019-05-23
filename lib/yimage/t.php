<?php
require("./yimage.php");
$data = [
    [
        "http://www.piyuedashi.com/uploads/answer-cards/20160817/9f302a86-9bfe-4a34-8b36-49ef2cb163b7.png",
        "http://www.piyuedashi.com/uploads/answer-cards/20160817/58bc303b-8514-42e3-a3d0-e1c74680468c.png"
    ],
    [
        "http://www.piyuedashi.com//uploads/raw-user-answers/2016-08-17/deskewed/b6e1039c-ddfa-45bb-aa7d-0c3696cfc484.jpg",
        "http://www.piyuedashi.com//uploads/raw-user-answers/2016-08-17/deskewed/2b41ec10-babc-40bd-b784-24cf6a51d7a8.jpg",
        "http://www.piyuedashi.com//uploads/raw-user-answers/2016-08-17/deskewed/f02c9157-943a-4bef-ad49-e2a9cde14447.jpg",
        "http://www.piyuedashi.com//uploads/raw-user-answers/2016-08-17/deskewed/f2c057f1-99c9-45c8-bdd9-6ed6ae4a1385.jpg"
    ]
];

$str = json_encode($data[0]);
$str0 = json_encode($data[1]);

$json = YImageClassify($str, $str0);
$data = json_decode($json);
var_dump($data);
foreach ($data as $item) {
	$name = $item->name;
	$rotation = $item->rotation;
	$image = imagecreatefromjpeg($name);
	$white = imagecolorallocate($image, 255, 255, 255);
	$img = imagerotate($image, $rotation, $white);
	imagejpeg($img, '/tmp/' . basename($name));
	imagedestroy($image);
}

