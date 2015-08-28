#include "testing/testing.hpp"

#include "generator/xml_element.hpp"
#include "generator/osm2type.hpp"

#include "routing/car_model.hpp"

#include "indexer/feature_data.hpp"
#include "indexer/classificator.hpp"
#include "indexer/classificator_loader.hpp"

#include "std/iostream.hpp"


namespace
{
  void FillXmlElement(char const * arr[][2], size_t count, XMLElement * p)
  {
    for (size_t i = 0; i < count; ++i)
      p->AddTag(arr[i][0], arr[i][1]);
  }

  template <size_t N> uint32_t GetType(char const * (&arr)[N])
  {
    vector<string> path(arr, arr + N);
    return classif().GetTypeByPath(path);
  }
  uint32_t GetType(StringIL const & lst) { return classif().GetTypeByPath(lst); }
}

UNIT_TEST(OsmType_SkipDummy)
{
  classificator::Load();

  char const * arr[][2] = {
    { "abutters", "residential" },
    { "highway", "primary" },
    { "osmarender:renderRef", "no" },
    { "ref", "E51" }
  };

  XMLElement e;
  FillXmlElement(arr, ARRAY_SIZE(arr), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL ( params.m_Types.size(), 1, (params) );
  TEST_EQUAL ( params.m_Types[0], GetType(arr[1]), () );
}


namespace
{
  void DumpTypes(vector<uint32_t> const & v)
  {
    Classificator const & c = classif();
    for (size_t i = 0; i < v.size(); ++i)
      cout << c.GetFullObjectName(v[i]) << endl;
  }

  void DumpParsedTypes(char const * arr[][2], size_t count)
  {
    XMLElement e;
    FillXmlElement(arr, count, &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    DumpTypes(params.m_Types);
  }
}

UNIT_TEST(OsmType_Check)
{
  classificator::Load();

  char const * arr1[][2] = {
    { "highway", "primary" },
    { "motorroad", "yes" },
    { "name", "Каширское шоссе" },
    { "oneway", "yes" }
  };

  char const * arr2[][2] = {
    { "highway", "primary" },
    { "name", "Каширское шоссе" },
    { "oneway", "-1" },
    { "motorroad", "yes" }
  };

  char const * arr3[][2] = {
    { "admin_level", "4" },
    { "border_type", "state" },
    { "boundary", "administrative" }
  };

  char const * arr4[][2] = {
    { "border_type", "state" },
    { "admin_level", "4" },
    { "boundary", "administrative" }
  };

  DumpParsedTypes(arr1, ARRAY_SIZE(arr1));
  DumpParsedTypes(arr2, ARRAY_SIZE(arr2));
  DumpParsedTypes(arr3, ARRAY_SIZE(arr3));
  DumpParsedTypes(arr4, ARRAY_SIZE(arr4));
}

UNIT_TEST(OsmType_Combined)
{
  char const * arr[][2] = {
    { "addr:housenumber", "84" },
    { "addr:postcode", "220100" },
    { "addr:street", "ул. Максима Богдановича" },
    { "amenity", "school" },
    { "building", "yes" },
    { "name", "Гимназия 15" }
  };

  XMLElement e;
  FillXmlElement(arr, ARRAY_SIZE(arr), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL(params.m_Types.size(), 2, (params));
  TEST(params.IsTypeExist(GetType(arr[3])), ());
  TEST(params.IsTypeExist(GetType({"building"})), ());

  string s;
  params.name.GetString(0, s);
  TEST_EQUAL(s, arr[5][1], ());

  TEST_EQUAL(params.house.Get(), "84", ());
}

UNIT_TEST(OsmType_Address)
{
  char const * arr[][2] = {
    { "addr:conscriptionnumber", "223" },
    { "addr:housenumber", "223/5" },
    { "addr:postcode", "11000" },
    { "addr:street", "Řetězová" },
    { "addr:streetnumber", "5" },
    { "source:addr", "uir_adr" },
    { "uir_adr:ADRESA_KOD", "21717036" }
  };

  XMLElement e;
  FillXmlElement(arr, ARRAY_SIZE(arr), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL(params.m_Types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"building", "address"})), ());

  TEST_EQUAL(params.house.Get(), "223/5", ());
}

UNIT_TEST(OsmType_PlaceState)
{
  char const * arr[][2] = {
    { "alt_name:vi", "California" },
    { "is_in", "USA" },
    { "is_in:continent", "North America" },
    { "is_in:country", "USA" },
    { "is_in:country_code", "us" },
    { "name", "California" },
    { "place", "state" },
    { "population", "37253956" },
    { "ref", "CA" }
  };

  XMLElement e;
  FillXmlElement(arr, ARRAY_SIZE(arr), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL(params.m_Types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"place", "state", "USA"})), ());

  string s;
  TEST(params.name.GetString(0, s), ());
  TEST_EQUAL(s, "California", ());
  TEST_GREATER(params.rank, 1, ());
}

UNIT_TEST(OsmType_AlabamaRiver)
{
  char const * arr1[][2] = {
    { "NHD:FCode", "55800" },
    { "NHD:FType", "558" },
    { "NHD:RESOLUTION", "2" },
    { "NHD:way_id", "139286586;139286577;139286596;139286565;139286574;139286508;139286600;139286591;139286507;139286505;139286611;139286602;139286594;139286604;139286615;139286616;139286608;139286514;139286511;139286564;139286576;139286521;139286554" },
    { "attribution", "NHD" },
    { "boat", "yes" },
    { "deep_draft", "no" },
    { "gnis:feature_id", "00517033" },
    { "name", "Tennessee River" },
    { "ship", "yes" },
    { "source", "NHD_import_v0.4_20100913205417" },
    { "source:deep_draft", "National Transportation Atlas Database 2011" },
    { "waterway", "river" }
  };

  char const * arr2[][2] = {
    { "destination", "Ohio River" },
    { "name", "Tennessee River" },
    { "type", "waterway" },
    { "waterway", "river" }
  };

  char const * arr3[][2] = {
    { "name", "Tennessee River" },
    { "network", "inland waterways" },
    { "route", "boat" },
    { "ship", "yes" },
    { "type", "route" }
  };

  XMLElement e;
  FillXmlElement(arr1, ARRAY_SIZE(arr1), &e);
  FillXmlElement(arr2, ARRAY_SIZE(arr2), &e);
  FillXmlElement(arr3, ARRAY_SIZE(arr3), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL(params.m_Types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"waterway", "river"})), ());
}

UNIT_TEST(OsmType_Synonyms)
{
  // Smoke test.
  {
    char const * arr[][2] = {
      { "building", "yes" },
      { "shop", "yes" },
      { "atm", "yes" },
      { "restaurant", "yes" },
      { "hotel", "yes" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    char const * arrT1[] = { "building" };
    char const * arrT2[] = { "amenity", "atm" };
    char const * arrT3[] = { "shop" };
    char const * arrT4[] = { "amenity", "restaurant" };
    char const * arrT5[] = { "tourism", "hotel" };
    TEST_EQUAL(params.m_Types.size(), 5, (params));

    TEST(params.IsTypeExist(GetType(arrT1)), ());
    TEST(params.IsTypeExist(GetType(arrT2)), ());
    TEST(params.IsTypeExist(GetType(arrT3)), ());
    TEST(params.IsTypeExist(GetType(arrT4)), ());
    TEST(params.IsTypeExist(GetType(arrT5)), ());
  }

  // Duplicating test.
  {
    char const * arr[][2] = {
      { "amenity", "atm" },
      { "atm", "yes" }
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"amenity", "atm"})), ());
  }

  // "NO" tag test.
  {
    char const * arr[][2] = {
      { "building", "yes" },
      { "shop", "no" },
      { "atm", "no" }
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"building"})), ());
  }
}

UNIT_TEST(OsmType_Capital)
{
  {
    char const * arr[][2] = {
      { "admin_level", "6" },
      { "capital", "yes" },
      { "place", "city" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"place", "city", "capital", "6"})), ());
  }

  {
    char const * arr[][2] = {
      { "admin_level", "6" },
      { "capital", "no" },
      { "place", "city" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"place", "city"})), ());
  }

  {
    char const * arr[][2] = {
      { "place", "city" },
      { "admin_level", "6" },
      { "boundary", "administrative" },
      { "capital", "2" },
      { "place", "city" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 2, (params));
    TEST(params.IsTypeExist(GetType({"place", "city", "capital", "2"})), ());
    TEST(params.IsTypeExist(GetType({"boundary", "administrative", "6"})), ());
  }
}

UNIT_TEST(OsmType_Route)
{
  {
    char const * arr[][2] = {
      { "highway", "motorway" },
      { "ref", "I 95" }
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
    TEST_EQUAL(params.ref, arr[1][1], ());
  }

  {
    char const * arr[][2] = {
      { "highway", "path" },
      { "ref", "route" }
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
    TEST(params.ref.empty(), ());
  }
}

UNIT_TEST(OsmType_Layer)
{
  {
    char const * arr[][2] = {
      { "highway", "motorway" },
      { "bridge", "yes" },
      { "layer", "2" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"highway", "motorway", "bridge"})), ());
    TEST_EQUAL(params.layer, 2, ());
  }

  {
    char const * arr[][2] = {
      { "highway", "trunk" },
      { "tunnel", "yes" },
      { "layer", "-1" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"highway", "trunk", "tunnel"})), ());
    TEST_EQUAL(params.layer, -1, ());
  }

  {
    char const * arr[][2] = {
      { "highway", "secondary" },
      { "bridge", "yes" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"highway", "secondary", "bridge"})), ());
    TEST_EQUAL(params.layer, 1, ());
  }

  {
    char const * arr[][2] = {
      { "highway", "primary" },
      { "tunnel", "yes" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"highway", "primary", "tunnel"})), ());
    TEST_EQUAL(params.layer, -1, ());
  }

  {
    char const * arr[][2] = {
      { "highway", "living_street" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
    TEST_EQUAL(params.layer, 0, ());
  }
}

UNIT_TEST(OsmType_Amenity)
{
  {
    char const * arr[][2] = {
      { "amenity", "bbq" },
      { "fuel", "wood" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
  }
}

UNIT_TEST(OsmType_Hwtag)
{
  char const * tags[][2] = {
      {"hwtag", "oneway"}, {"hwtag", "private"}, {"hwtag", "lit"}, {"hwtag", "nofoot"}, {"hwtag", "yesfoot"},
  };

  {
    char const * arr[][2] = {
      { "railway", "rail" },
      { "access", "private" },
      { "oneway", "true" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
  }

  {
    char const * arr[][2] = {
        {"oneway", "-1"},
        {"highway", "primary"},
        {"access", "private"},
        {"lit", "no"},
        {"foot", "no"},
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 4, (params));
    TEST(params.IsTypeExist(GetType(arr[1])), ());
    TEST(params.IsTypeExist(GetType(tags[0])), ());
    TEST(params.IsTypeExist(GetType(tags[1])), ());
    TEST(params.IsTypeExist(GetType(tags[3])), ());
  }

  {
    char const * arr[][2] = {
        {"foot", "yes"}, {"highway", "primary"},
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 2, (params));
    TEST(params.IsTypeExist(GetType(arr[1])), ());
    TEST(params.IsTypeExist(GetType(tags[4])), ());
  }
}

UNIT_TEST(OsmType_Ferry)
{
  routing::CarModel carModel;

  char const * arr[][2] = {
    { "motorcar", "yes" },
    { "highway", "primary" },
    { "bridge", "yes" },
    { "route", "ferry" },
  };

  XMLElement e;
  FillXmlElement(arr, ARRAY_SIZE(arr), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL(params.m_Types.size(), 2, (params));

  uint32_t type = GetType({"highway", "primary", "bridge"});
  TEST(params.IsTypeExist(type), ());
  TEST(carModel.IsRoad(type), ());

  type = GetType({"route", "ferry", "motorcar"});
  TEST(params.IsTypeExist(type), ());
  TEST(carModel.IsRoad(type), ());

  type = GetType({"route", "ferry"});
  TEST(!params.IsTypeExist(type), ());
  TEST(!carModel.IsRoad(type), ());
}

UNIT_TEST(OsmType_Boundary)
{
  char const * arr[][2] = {
    { "admin_level", "6" },
    { "boundary", "administrative" },
    { "admin_level", "2" },
    { "boundary", "administrative" },
  };

  XMLElement e;
  FillXmlElement(arr, ARRAY_SIZE(arr), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL(params.m_Types.size(), 2, (params));
  TEST(params.IsTypeExist(GetType({"boundary", "administrative", "2"})), ());
  TEST(params.IsTypeExist(GetType({"boundary", "administrative", "6"})), ());
}

UNIT_TEST(OsmType_Dibrugarh)
{
  char const * arr[][2] = {
    { "AND_a_c", "10001373" },
    { "addr:city", "Dibrugarh" },
    { "addr:housenumber", "hotel vishal" },
    { "addr:postcode", "786001" },
    { "addr:street", "Marwari Patty,Puja Ghat" },
    { "name", "Dibrugarh" },
    { "phone", "03732320016" },
    { "place", "city" },
    { "website", "http://www.hotelvishal.in" },
  };

  XMLElement e;
  FillXmlElement(arr, ARRAY_SIZE(arr), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL(params.m_Types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"place", "city"})), (params));
  string name;
  TEST(params.name.GetString(StringUtf8Multilang::DEFAULT_CODE, name), (params));
  TEST_EQUAL(name, "Dibrugarh", (params));
}

UNIT_TEST(OsmType_Subway)
{
  {
    char const * arr[][2] = {
      { "network", "Московский метрополитен" },
      { "operator", "ГУП «Московский метрополитен»" },
      { "railway", "station" },
      { "station", "subway" },
      { "transport", "subway" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "subway", "moscow"})), (params));
  }

  {
    char const * arr[][2] = {
      { "name", "14th Street-8th Avenue (A,C,E,L)" },
      { "network", "New York City Subway" },
      { "railway", "station" },
      { "wheelchair", "yes" },
      { "route", "subway" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "subway", "newyork"})), (params));
  }

  {
    char const * arr[][2] = {
      { "name", "S Landsberger Allee" },
      { "phone", "030 29743333" },
      { "public_transport", "stop_position" },
      { "railway", "station" },
      { "network", "New York City Subway" },
      { "station", "light_rail" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "light_rail"})), (params));
  }

  {
    char const * arr[][2] = {
      { "monorail", "yes" },
      { "name", "Улица Академика Королёва" },
      { "network", "Московский метрополитен" },
      { "operator", "ГУП «Московский метрополитен»" },
      { "public_transport", "stop_position" },
      { "railway", "station" },
      { "station", "monorail" },
      { "transport", "monorail" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "monorail"})), (params));
  }

  {
    char const * arr[][2] = {
      { "line",	"Northern, Bakerloo" },
      { "name",	"Charing Cross" },
      { "network", "London Underground" },
      { "operator", "TfL" },
      { "railway", "station" },
    };

    XMLElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_Types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "subway", "london"})), (params));
  }
}
