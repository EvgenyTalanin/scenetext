<?php
	function cmp($a, $b)
	{
		if ($a["box"] > $b["box"]) return 1;
		else if ($a["box"] < $b["box"]) return -1;
		else if ($a["area"] > $b["area"]) return 1;
		else if ($a["area"] < $b["area"]) return -1;
		else if ($a["perimeter"] > $b["perimeter"]) return 1;
		else if ($a["perimeter"] < $b["perimeter"]) return -1;
		else if ($a["euler"] > $b["euler"]) return 1;
		else if ($a["euler"] < $b["euler"]) return -1;
		else if ($a["crossings"] > $b["crossings"]) return 1;
		else if ($a["crossings"] < $b["crossings"]) return -1;
		return 0;
	}


	$files_dir = "/home/evgeny/argus/scenetext/testimages/";
	$filenames = array("ontario_small.jpg", "vilnius.jpg", "lines.jpg", "painting.jpg", "road.jpg", "floor.jpg", "campaign.jpg", "incorrect640.jpg", "lettera.jpg", "abv.jpg");
	$exename = "bin/scenetext";
	$op = array();
	$code = 0;
	$results = array();
	$times = array();
	$approach_idx = 0;
	$region_idx = 0;

	foreach($filenames as $fn)
	{
		echo "Testing {$fn}... ";

		$approach_idx = 0;
		$region_idx = 0;
		$op = array();
		$results = array();

		exec("{$exename} {$files_dir}{$fn}", $op, $code);
		if ($code != 0)
		{
			die("FAIL\n");
		}

		foreach($op as $l)
		{
			if (strpos($l, "time") !== false)
			{
				$times[$approach_idx] = floatval(substr($l, strpos($l, "time: ") + 6));
				$approach_idx++;
				$region_idx = 0;
			}
			else if (strpos($l, "New region") !== false)
			{
				$region_idx++;
			}
			else if (strpos($l, "Area") !== false)
			{
				$results[$approach_idx][$region_idx]["area"] = intval(substr($l, strpos($l, ": ") + 2));
			}
			else if (strpos($l, "Bounding box") !== false)
			{
				$results[$approach_idx][$region_idx]["box"] = trim(substr($l, strpos($l, "(")));
			}
			else if (strpos($l, "Perimeter") !== false)
			{
				$results[$approach_idx][$region_idx]["perimeter"] = intval(substr($l, strpos($l, ": ") + 2));
			}
			else if (strpos($l, "Euler number") !== false)
			{
				$results[$approach_idx][$region_idx]["euler"] = intval(substr($l, strpos($l, ": ") + 2));
			}
			else if (strpos($l, "Crossings") !== false)
			{
				$results[$approach_idx][$region_idx]["crossings"] = trim(substr($l, strpos($l, ": ") + 2));
			}
			else if (strpos($l, "fault") !== false)
			{
				die("FAIL");
			}
		}

		usort($results[0], "cmp");
		usort($results[1], "cmp");

		if (count($results[0]) != count($results[1]))
		{
			echo "FAIL (number of elements)";
		}
		else
		{
			for($i = 0; $i < count($results[0]); $i++)
			{
				if ( ($results[0][$i]["area"] != $results[1][$i]["area"]) ||
				     ($results[0][$i]["box"] != $results[1][$i]["box"]) ||
				     ($results[0][$i]["perimeter"] != $results[1][$i]["perimeter"]) ||
				     ($results[0][$i]["euler"] != $results[1][$i]["euler"]) ||
				     ($results[0][$i]["crossings"] != $results[1][$i]["crossings"]) )
				{
					echo "FAIL (element #{$i})";
					print_r($results[0][$i]);
					print_r($results[1][$i]);
					goto mark1;
				}
			}

			echo "SUCCESS ({$times[0]} vs. {$times[1]} ms): " . round($times[1] / $times[0], 2) . " times slower";
		}
		mark1: echo "\n";
	}
?>
