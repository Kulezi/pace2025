for gr in ../.solutions/**/*.sol; do
    echo $gr
    rm $(dirname $gr)/*.gexf
    python convert_ads_to_gexf.py $gr ${gr%.sol}.gexf
    for i in $(seq 5); do
        python gexf_subgraph.py $gr ${gr%.sol}_random_subgraph$i.gexf
    done
done