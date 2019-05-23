<?php
require("./image_tool.php");
$json = "[\"./a.jpg\", \"./b.jpg\"]";
$template = [
	"http://www.yijiafen.com//uploads/answer-cards/20170116/6391dc34-a0d3-478f-95ac-4f2226558976.png"
];
$image = [
	"http://www.yijiafen.com//uploads/raw-user-answers/2017-01-16/deskewed/524acbe9-a2e3-4225-b51c-d45abf2b4056.jpg"
];
$ret = image_tool::classify_by_line(json_encode($template), json_encode($image));
var_dump(json_decode($ret));
