<?php
require("./yimage.php");
$data = [
		[
			'id'		=> 'good',
			'filename'	=> "http://www.piyuedashi.com/uploads/answer-cards/20160817/9f302a86-9bfe-4a34-8b36-49ef2cb163b7.png",
			'x' 		=> 0,
			'y' 		=> 0,
			'w' 		=> 1,
			'h' 		=> 0.5,
		],
		[
			'id'		=> 'nice',
			'filename'	=> "http://www.piyuedashi.com/uploads/answer-cards/20160817/9f302a86-9bfe-4a34-8b36-49ef2cb163b7.png",
			'x' 		=> 0,
			'y' 		=> 0.5,
			'w' 		=> 1,
			'h' 		=> 1,
		],
//		[
//			'filename'	=> "/home/yjf/entropy.jpg",
//			'x' 		=> 0,
//			'y' 		=> 0,
//			'w' 		=> 1,
//			'h' 		=> 0.5,
//		],
//		[
//			'filename'	=> "/home/yjf/entropy.jpg",
//			'x' 		=> 0,
//			'y' 		=> 0.5,
//			'w' 		=> 1,
//			'h' 		=> 1,
//		],
//		[
//			'filename'	=> "/home/yjf/resized.png",
//			'x' 		=> 0,
//			'y' 		=> 0,
//			'w' 		=> 1,
//			'h' 		=> 0.5,
//		],
//		[
//			'filename'	=> "/home/yjf/resized.png",
//			'x' 		=> 0,
//			'y' 		=> 0.5,
//			'w' 		=> 1,
//			'h' 		=> 1,
//		],
//		[
//			'filename'	=> "/home/yjf/snapshot.png",
//			'x' 		=> 0,
//			'y' 		=> 0,
//			'w' 		=> 1,
//			'h' 		=> 0.5,
//		],
//		[
//			'filename'	=> "/home/yjf/snapshot.png",
//			'x' 		=> 0,
//			'y' 		=> 0.5,
//			'w' 		=> 1,
//			'h' 		=> 1,
//		],
//        "http://www.piyuedashi.com/uploads/answer-cards/20160817/58bc303b-8514-42e3-a3d0-e1c74680468c.png"
];

$str = json_encode($data);
$json = YImageEntropy($str);
echo $json;
