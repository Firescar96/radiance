# while :; do
#   for hue in 8192 40960 16384 49152 24576 65535 32768 57344; do
#     echo $hue
#     curl -XPUT -d '{"hue":'$hue',"sat":150,"bri":150, "transitiontime":0}' http://192.168.1.198/api/o6qkXc02MVbrYID9PqGT5r5Yjq0WVrXZ1t3kvdKG/lights/1/state
#     sleep .001
#   done
# done

while :; do
  for hue in 24576 -24576 24576 -24576 24576 -24576 24576; do
    echo $hue
    curl -XPOST -d '{"sat_inc":0,"hue_inc":'$hue',"bri_inc":0,"transitiontime":0}' http://192.168.1.198/api/o6qkXc02MVbrYID9PqGT5r5Yjq0WVrXZ1t3kvdKG/lights/1/state
    sleep .001
  done
done
