# Applies renaming within all of the rapidjson source files to add a ser20 prefix
find ./../include/ser20/external/rapidjson/ -type f -name \*.h -exec sed -i "s/RAPIDJSON_/SER20_RAPIDJSON_/g" {} \;
echo "Remember to backport any ser20 specific changes not in this version of RapidJSON!"
echo "See https://github.com/USCiLab/ser20/commits/develop/include/ser20/external/rapidjson"
