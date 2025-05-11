for gr in ../.solutions/**/*.sol; do
    echo $gr
    rm $(dirname $gr)/*.gexf
    python convert_ads_to_gexf.py $gr ${gr%.sol}.gexf 99999999
    for i in $(seq 5); do
        python convert_ads_to_gexf.py $gr ${gr%.sol}_random_subgraph$i.gexf $i
    done
done
