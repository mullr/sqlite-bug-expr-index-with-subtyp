#include <stdio.h>
#include <sqlite3.h>
#include <assert.h>

#define SUBTYPE_HAS_SEEDS 1
#define SUBTYPE_SEEDLESS 2

static char *table_schema =
  "create table fruit (\n"
  "  id integer not null,\n"
  "  name text not null,\n"
  "  seed_count integer not null\n"
  ")";

static char* inserts[] = {
  "insert into fruit values (0, 'apple', 12);",
  "insert into fruit values (1, 'orange', 6);",
  "insert into fruit values (2, 'grape', 1);",
  "insert into fruit values (3, 'seedless grape', 0);",
  "insert into fruit values (4, 'watermelon', 1000);",
  "insert into fruit values (5, 'seedless watermelon', 0);",
};

static char* select_seeded_fruits =
  "select name from fruit where fruit_has_seeds(pack_fruit(name, seed_count));";


static char* packed_fruit_index =
  "create index packed_fruit_index on fruit(name, pack_fruit(name, seed_count))";


/* static char *seediness_index = */
/*   "create index fruit_seediness on fruit()" */


static void packFruitFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
    assert(argc == 2);
    sqlite3_result_value(context, argv[0]);

    int seed_count = sqlite3_value_int(argv[1]);
    if (seed_count > 0) {
      sqlite3_result_subtype(context, SUBTYPE_HAS_SEEDS);
    } else {
      sqlite3_result_subtype(context, SUBTYPE_SEEDLESS);
    }
}

static void hasSeedsFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
    assert(argc == 1);
    int subtype = sqlite3_value_subtype(argv[0]);
    switch (subtype) {
    case SUBTYPE_HAS_SEEDS:
      sqlite3_result_int(context, 1);
      break;

    case SUBTYPE_SEEDLESS:
      sqlite3_result_int(context, 0);
      break;

    default:
      fprintf(stderr, "Unknown subtype: %i\n", subtype);
      assert(0);
    }
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int main(int argc, char **argv){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;

  rc = sqlite3_open("fruit.db", &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  rc = sqlite3_exec(db, "select sqlite_version()", callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  rc = sqlite3_exec(db, table_schema, NULL , 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  static const int flags = SQLITE_UTF8 | SQLITE_INNOCUOUS | SQLITE_DETERMINISTIC;
  rc = sqlite3_create_function(db, "pack_fruit", 2, flags, NULL, packFruitFunc, 0, 0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  rc = sqlite3_create_function(db, "fruit_has_seeds", 1, flags, NULL, hasSeedsFunc, 0, 0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  
  int insert_count = sizeof(inserts) / sizeof(inserts[0]);
  for (int i=0; i<insert_count; i++) {
    char* insert = inserts[i];
    rc = sqlite3_exec(db, insert, NULL, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
  }

  printf("**** Query results with no index: ****\n");
  rc = sqlite3_exec(db, select_seeded_fruits, callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  rc = sqlite3_exec(db, packed_fruit_index, NULL , 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  /* rc = sqlite3_exec(db, "analyze", NULL , 0, &zErrMsg); */
  /* if( rc!=SQLITE_OK ){ */
  /*   fprintf(stderr, "SQL error: %s\n", zErrMsg); */
  /*   sqlite3_free(zErrMsg); */
  /* } */

  printf("**** Query results with index: ****\n");
  rc = sqlite3_exec(db, select_seeded_fruits, callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  sqlite3_close(db);
  return 0;
}
