#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <map>
#include <set>
#include <unordered_set>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <optional>

using namespace std;

/* ========== Saad DB ==========



  Storage:
    - Schema file: SaadSchema.txt
      Blocks:
        *TableName*
        <<
        pk: primaryKeyName
        col1 <type...>
        col2 <type...>
        ...
        >>
    - Table data: <TableName>.sdb
      Lines like: <v1,v2,...,vn>

  Notes:
    - Case-insensitive keywords; identifiers & values keep case.
    - Strings in INSERT must be quoted "like this".
    - WHERE supports basic comparisons:
        numeric: = != < >
        string : = !=
    - Types: int, varchar N, date (dd-mm-yyyy), decimal P S
*/

static const string SCHEMA_FILE = "SaadSchema.txt";
vector<string> TOKENS;
vector<string> ATTRS;


static inline string trim(const string& s) {
    size_t i = 0, j = s.size();
    while (i < j && isspace((unsigned char)s[i])) ++i;
    while (j > i && isspace((unsigned char)s[j - 1])) --j;
    return s.substr(i, j - i);
}
static inline bool starts_with(const string& s, char c) { return !s.empty() && s.front() == c; }
static inline bool ends_with(const string& s, char c) { return !s.empty() && s.back() == c; }

static bool safe_getline(ifstream& f, string& out) {
    out.clear();
    return static_cast<bool>(std::getline(f, out));
}
static bool safe_getline(fstream& f, string& out) {
    out.clear();
    return static_cast<bool>(std::getline(f, out));
}

static bool is_integer(const string& s) {
    if (s.empty()) return false;
    size_t i = 0; if (s[0] == '+' || s[0] == '-') i = 1;
    if (i >= s.size()) return false;
    for (; i < s.size(); ++i) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}
static bool is_number(const string& s) {

    if (s.empty()) return false;
    bool dot = false; size_t i = 0;
    if (s[0] == '+' || s[0] == '-') i = 1;
    if (i >= s.size()) return false;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') { if (dot) return false; dot = true; }
        else if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

static vector<string> split_csv_inside_tuple(const string& tupleLine) {

    vector<string> res;
    if (tupleLine.size() < 2 || tupleLine.front() != '<' || tupleLine.back() != '>') return res;
    string body = tupleLine.substr(1, tupleLine.size() - 2);


    string cur;
    for (char c : body) {
        if (c == ',') { res.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    if (!cur.empty()) res.push_back(cur);
    return res;
}

static string join_csv_tuple(const vector<string>& vals) {
    string s = "<";
    for (size_t i = 0; i < vals.size(); ++i) {
        s += vals[i];
        if (i + 1 < vals.size()) s += ',';
    }
    s += ">";
    return s;
}

static bool file_exists(const string& path) {
    ifstream f(path);
    return f.good();
}


static bool table_exists(const string& table) {
    ifstream in(SCHEMA_FILE);
    if (!in) return false;
    string line;
    while (safe_getline(in, line)) {
        line = trim(line);
        if (starts_with(line, '*') && ends_with(line, '*')) {
            string name = line.substr(1, line.size() - 2);
            if (name == table) return true;
        }
    }
    return false;
}

static bool read_table_block(const string& table, vector<string>& block) {
    block.clear();
    ifstream in(SCHEMA_FILE);
    if (!in) return false;
    string line;
    bool inBlock = false, found = false;
    while (safe_getline(in, line)) {
        string t = trim(line);
        if (!inBlock) {
            if (starts_with(t, '*') && ends_with(t, '*')) {
                string name = t.substr(1, t.size() - 2);
                if (name == table) {

                    found = true;
                    block.push_back(t);
                    while (safe_getline(in, line)) {
                        block.push_back(line);
                        if (trim(line) == ">>") break;
                    }
                    break;
                }
            }
        }
    }
    return found;
}

static bool get_pk_of(const string& table, string& pk) {
    vector<string> blk;
    if (!read_table_block(table, blk)) return false;

    for (const auto& ln : blk) {
        string t = trim(ln);
        if (t.rfind("pk:", 0) == 0) {
            pk = trim(t.substr(3));
            return true;
        }
    }
    return false;
}

static bool fill_attrs_of(const string& table, vector<string>& attrs) {
    attrs.clear();
    vector<string> blk;
    if (!read_table_block(table, blk)) return false;
    bool after_ll = false;
    for (const auto& ln : blk) {
        string t = trim(ln);
        if (t == "<<") { after_ll = true; continue; }
        if (t == ">>") break;
        if (!after_ll) continue;
        if (t.rfind("pk:", 0) == 0) continue;

        if (t.empty()) continue;
        string name;
        {
            istringstream ss(t);
            ss >> name;
        }
        if (!name.empty()) attrs.push_back(name);
    }
    return !attrs.empty();
}

static int count_attrs(const string& table) {
    vector<string> a;
    if (!fill_attrs_of(table, a)) return 0;
    return (int)a.size();
}

static bool append_table_schema(const string& table, const vector<string>& colLines, const string& pk) {
    ofstream out(SCHEMA_FILE, ios::app);
    if (!out) return false;
    out << "*" << table << "*\n";
    out << "<<\n";
    out << "pk: " << pk << "\n";
    for (const auto& ln : colLines) out << ln << "\n";
    out << ">>\n\n";
    return true;
}

static bool remove_table_schema(const string& table) {
    ifstream in(SCHEMA_FILE);
    if (!in) return false;
    string all((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    in.close();


    string start = "*" + table + "*\n";
    size_t pos = all.find(start);
    if (pos == string::npos) return false;

    size_t blockStart = pos;
    size_t after = all.find("\n\n", blockStart);
    if (after == string::npos) return false;
    all.erase(blockStart, after - blockStart + 2);

    ofstream out(SCHEMA_FILE, ios::trunc);
    if (!out) return false;
    out << all;
    return true;
}


static const unordered_set<string> KEYWORDS = {
    "create","table","primary","key","int","varchar","date","decimal",
    "drop","describe","insert","into","values","help","tables","select",
    "from","where","and","or","update","set","delete","quit"
};

static void parse_tokens(const string& q) {
    TOKENS.clear();
    string temp;
    for (size_t i = 0; i < q.size(); ++i) {
        char c = q[i];
        if (c == '"') {
            string s;
            ++i;
            while (i < q.size() && q[i] != '"') { s.push_back(q[i]); ++i; }
            TOKENS.push_back(s);
        }
        else if (c == ' ' || c == '(' || c == ')' || c == ',' || c == ';') {
            if (!temp.empty()) { TOKENS.push_back(temp); temp.clear(); }
            if (c == '*') TOKENS.push_back("*");
        }
        else if (c == '!' && i + 1 < q.size() && q[i + 1] == '=') {
            if (!temp.empty()) { TOKENS.push_back(temp); temp.clear(); }
            TOKENS.push_back("!="); ++i;
        }
        else if (c == '<' || c == '>' || c == '=') {
            if (!temp.empty()) { TOKENS.push_back(temp); temp.clear(); }
            TOKENS.push_back(string(1, c));
        }
        else {
            temp.push_back(c);
        }
    }
    if (!temp.empty()) TOKENS.push_back(temp);


    for (string& t : TOKENS) {
        string low = t;
        transform(low.begin(), low.end(), low.begin(), ::tolower);
        if (KEYWORDS.count(low)) t = low;
    }
}

static bool has_semicolon(const string& q) {
    for (int i = (int)q.size() - 1; i >= 0; --i) {
        if (!isspace((unsigned char)q[i])) return q[i] == ';';
    }
    return false;
}


static bool is_date_token(const string& s) {



    if (s.size() != 10) return false;
    return isdigit(s[0]) && isdigit(s[1]) && s[2] == '-' &&
        isdigit(s[3]) && isdigit(s[4]) && s[5] == '-' &&
        isdigit(s[6]) && isdigit(s[7]) && isdigit(s[8]) && isdigit(s[9]);
}

static string infer_token_type(const string& t) {
    if (isalpha((unsigned char)t[0])) return "varchar";
    if (is_date_token(t)) return "date";
    if (is_integer(t)) return "int";
    if (is_number(t)) return "decimal";
    return "varchar"; // fallback
}


static bool ensure_table_exists(const string& name) {
    if (!table_exists(name)) {
        cout << "[SaadDB] table <" << name << "> doesn't exist\n";
        return false;
    }
    return true;
}
static bool ensure_table_absent(const string& name) {
    if (table_exists(name)) {
        cout << "[SaadDB] table <" << name << "> already exists\n";
        return false;
    }
    return true;
}


static void cmd_help(const vector<string>& T) {
    if (T.size() == 1) {
        cout << "Saad DB Help:\n"
            "  help tables;\n"
            "  help create; help drop; help insert; help select; help update; help delete;\n";
        return;
    }
    if (T.size() >= 2) {
        string k = T[1];
        if (k == "tables") {
            ifstream in(SCHEMA_FILE);
            if (!in) { cout << "[SaadDB] No schema yet.\n"; return; }
            string line; bool any = false;
            cout << "Tables:\n";
            while (safe_getline(in, line)) {
                string t = trim(line);
                if (starts_with(t, '*') && ends_with(t, '*')) {
                    cout << "  " << t.substr(1, t.size() - 2) << "\n"; any = true;
                }
            }
            if (!any) cout << "  (none)\n";
            return;
        }
        if (k == "create") {
            cout << "create table T(a int, b varchar(30), d date, x decimal(7,2), primary key(a));\n";
            return;
        }
        if (k == "drop") { cout << "drop table T;\n"; return; }
        if (k == "insert") { cout << "insert into T values(1,\"Name\",24-02-2001,500.25);\n"; return; }
        if (k == "select") { cout << "select * from T where a>10;  select a,b from T where b!=\"x\";\n"; return; }
        if (k == "update") { cout << "update T set b=\"Z\", x=123.45 where a=1;\n"; return; }
        if (k == "delete") { cout << "delete from T where a!=5;\n"; return; }
        cout << "[SaadDB] Unknown help topic.\n";
    }
}

static void cmd_create(const vector<string>& T) {

    if (T.size() < 4 || T[0] != "create" || T[1] != "table") { cout << "[SaadDB] INVALID CREATE\n"; return; }
    string table = T[2];
    if (!ensure_table_absent(table)) { cout << "[SaadDB] Table not created\n"; return; }


    int n = (int)T.size();
    int pPrimary = -1;
    for (int i = 3; i < n; i++) if (T[i] == "primary") { pPrimary = i; break; }
    if (pPrimary == -1 || pPrimary + 3 >= n || T[pPrimary + 1] != "key") {
        cout << "[SaadDB] Defining primary key is mandatory. Table not created.\n"; return;
    }
    string pk = T[pPrimary + 2]; 
    vector<string> colLines;
    for (int i = 3; i < pPrimary; ) {

        if (i >= pPrimary) break;
        string name = T[i++];
        if (i >= pPrimary) { cout << "[SaadDB] Column type missing for " << name << "\n"; return; }
        string type = T[i++]; // int|varchar|date|decimal
        ostringstream ln; ln << name << " " << type;

        if (type == "varchar") {
            if (i >= pPrimary) { cout << "[SaadDB] varchar requires length\n"; return; }
            ln << " " << T[i++]; // length
        }
        else if (type == "decimal") {
            if (i + 1 >= pPrimary) { cout << "[SaadDB] decimal requires P S\n"; return; }
            ln << " " << T[i++] << " " << T[i++]; // P S
        }
        else if (type == "int" || type == "date") {

        }
        else {
            cout << "[SaadDB] Unknown type " << type << "\n"; return;
        }


        if (i < pPrimary && T[i] == "check") {
            ln << " check";
            while (i < pPrimary && T[i] != "primary") { ln << " " << T[i++]; }
        }
        colLines.push_back(ln.str());
    }

    if (colLines.empty()) { cout << "[SaadDB] No columns.\n"; return; }

    if (!append_table_schema(table, colLines, pk)) { cout << "[SaadDB] Failed writing schema.\n"; return; }

    ofstream tf(table + ".sdb", ios::app);
    tf.close();

    cout << "[SaadDB] Table <" << table << "> created successfully.\n";
}

static void cmd_drop(const vector<string>& T) {

    if (T.size() < 3) { cout << "[SaadDB] INVALID DROP\n"; return; }
    string table = T[2];
    if (!ensure_table_exists(table)) { cout << "[SaadDB] Table not dropped\n"; return; }


    remove((table + ".sdb").c_str());

    if (!remove_table_schema(table)) { cout << "[SaadDB] Failed to update schema\n"; return; }
    cout << "[SaadDB] <" << table << "> dropped successfully.\n";
}

static void cmd_describe(const vector<string>& T) {

    if (T.size() < 2) { cout << "[SaadDB] INVALID DESCRIBE\n"; return; }
    string table = T[1];
    if (!ensure_table_exists(table)) return;

    vector<string> blk;
    if (!read_table_block(table, blk)) { cout << "[SaadDB] Corrupt schema.\n"; return; }
    for (const auto& ln : blk) cout << ln << "\n";
}

static bool read_schema_types(const string& table, vector<string>& types) {
    types.clear();
    vector<string> blk;
    if (!read_table_block(table, blk)) return false;
    bool in = false;
    for (const auto& ln : blk) {
        string t = trim(ln);
        if (t == "<<") { in = true; continue; }
        if (t == ">>") break;
        if (!in) continue;
        if (t.rfind("pk:", 0) == 0) continue;
        istringstream ss(t);
        string name, typ; ss >> name >> typ;
        types.push_back(typ);
    }
    return !types.empty();
}

static bool check_values_match_schema(const vector<string>& vals, const vector<string>& types) {
    if (vals.size() != types.size()) return false;
    for (size_t i = 0; i < types.size(); ++i) {
        const string& ty = types[i];
        const string& v = vals[i];
        if (ty == "int") {
            if (!is_integer(v)) return false;
        }
        else if (ty == "date") {
            if (!is_date_token(v)) return false;
        }
        else if (ty == "decimal") {
            if (!is_number(v)) return false;
        }
        else if (ty == "varchar") {


        }
        else {
            return false;
        }
    }
    return true;
}

static void cmd_insert(const vector<string>& T) {

    if (T.size() < 4 || T[0] != "insert" || T[1] != "into") { cout << "[SaadDB] INVALID INSERT\n"; return; }
    string table = T[2];
    if (!ensure_table_exists(table)) { cout << "[SaadDB] Tuple not inserted\n"; return; }


    int i = 3; while (i < (int)T.size() && T[i] != "values") ++i;
    if (i >= (int)T.size() - 1) { cout << "[SaadDB] VALUES missing.\n"; return; }
    ++i; 


    vector<string> vals;
    for (; i < (int)T.size(); ++i) vals.push_back(T[i]);



    vector<string> types;
    if (!read_schema_types(table, types)) { cout << "[SaadDB] Schema types read error.\n"; return; }


    string pk; if (!get_pk_of(table, pk)) { cout << "[SaadDB] PK missing in schema.\n"; return; }
    vector<string> attrs; fill_attrs_of(table, attrs);
    int pkIndex = find(attrs.begin(), attrs.end(), pk) - attrs.begin();
    if (pkIndex < 0 || pkIndex >= (int)attrs.size()) { cout << "[SaadDB] PK not in attrs.\n"; return; }


    if (vals.size() != attrs.size()) {
        cout << "[SaadDB] Values count mismatch. Expected " << attrs.size() << ", got " << vals.size() << "\n";
        return;
    }

    if (!check_values_match_schema(vals, types)) {
        cout << "[SaadDB] Values don't match column types.\n"; return;

    }




    ifstream in(table + ".sdb");
    string line;
    while (safe_getline(in, line)) {
        if (line.empty()) continue;
        auto v = split_csv_inside_tuple(line);
        if (v.size() == attrs.size()) {
            if (v[pkIndex] == vals[pkIndex]) {
                cout << "[SaadDB] PK already exists.\n"; return;
            }
        }
    }
    in.close();

    ofstream out(table + ".sdb", ios::app);
    out << join_csv_tuple(vals) << "\n";
    cout << "[SaadDB] Tuple inserted successfully.\n";
}

static bool eval_where(const vector<string>& row, const vector<string>& attrs,
    const vector<string>& T, int fromPos) {


    int i = fromPos + 1;
    if (i >= (int)T.size() || T[i] != "where") return true;



    auto eval_one = [&](int j) -> int {
        if (j + 3 > (int)T.size() - 1) return -1;
        string col = T[j], op = T[j + 1], val = T[j + 2];
        int idx = find(attrs.begin(), attrs.end(), col) - attrs.begin();
        if (idx < 0 || idx >= (int)attrs.size()) return -1;
        const string& r = row[idx];

        bool valIsNum = is_number(val);
        bool rowIsNum = is_number(r);

        if (valIsNum && rowIsNum) {
            long double a = stold(r), b = stold(val);
            if (op == "=")  return a == b;
            if (op == "!=") return a != b;
            if (op == ">")  return a > b;
            if (op == "<")  return a < b;
            return -1;
        }
        else {
            if (op == "=")  return r == val;
            if (op == "!=") return r != val;
            return -1;
        }
        };


    bool acc = false; bool have = false; string pending = "";
    for (int j = i + 1; j < (int)T.size(); ) {
        int r = eval_one(j);
        if (r == -1) return false;
        if (!have) { acc = (bool)r; have = true; }

    else {
            if (pending == "and") acc = acc && r;
            else if (pending == "or") acc = acc || r;
            else return false;
        }
        j += 3;
        if (j < (int)T.size()) {
            if (T[j] == "and" || T[j] == "or") { pending = T[j]; ++j; }
            else break;
        }
    }
    return have ? acc : true;
}

static void cmd_select(const vector<string>& T) {

    if (T.size() < 4 || T[0] != "select") { cout << "[SaadDB] INVALID SELECT\n"; return; }

    int i = 1;
    vector<string> want;
    for (; i < (int)T.size() && T[i] != "from"; ++i) {
        if (T[i] == ",") continue;
        want.push_back(T[i]);
    }
    if (i >= (int)T.size() - 1 || T[i] != "from") { cout << "[SaadDB] FROM missing\n"; return; }
    string table = T[++i];
    if (!ensure_table_exists(table)) return;

    vector<string> attrs; fill_attrs_of(table, attrs);
    bool select_all = (want.size() == 1 && want[0] == "*");
    vector<int> idxs;
    if (!select_all) {
        for (const auto& w : want) {
            auto it = find(attrs.begin(), attrs.end(), w);
            if (it == attrs.end()) { cout << "[SaadDB] Unknown column " << w << "\n"; return; }
            idxs.push_back((int)(it - attrs.begin()));
        }
    }

    ifstream in(table + ".sdb");
    if (!in) { cout << "[SaadDB] No data.\n"; return; }
    string line; bool any = false;
    while (safe_getline(in, line)) {
        if (line.empty()) continue;
        auto row = split_csv_inside_tuple(line);
        if (row.size() != attrs.size()) continue;
        if (!eval_where(row, attrs, T, i)) continue;
        any = true;
        if (select_all) {
            for (const auto& v : row) cout << left << setw(20) << v;
        }
        else {
            for (int k : idxs) cout << left << setw(20) << row[k];
        }
        cout << "\n";
    }
    if (!any) cout << "[SaadDB] (no rows)\n";
}

static void cmd_update(const vector<string>& T) {

    if (T.size() < 4 || T[0] != "update") { cout << "[SaadDB] INVALID UPDATE\n"; return; }
    string table = T[1];
    if (!ensure_table_exists(table)) return;

    int pSet = (int)(find(T.begin(), T.end(), "set") - T.begin());
    if (pSet == (int)T.size()) { cout << "[SaadDB] SET missing\n"; return; }

    vector<string> attrs; fill_attrs_of(table, attrs);
    string pk; get_pk_of(table, pk);


    map<int, string> updates; 

    for (int i = pSet + 1; i < (int)T.size(); ) {
        if (T[i] == "where") break;
        if (i + 2 >= (int)T.size() || T[i + 1] != "=") { cout << "[SaadDB] Bad assignment\n"; return; }
        string col = T[i], val = T[i + 2];
        int idx = find(attrs.begin(), attrs.end(), col) - attrs.begin();
        if (idx < 0 || idx >= (int)attrs.size()) { cout << "[SaadDB] Unknown column " << col << "\n"; return; }
        updates[idx] = val;
        i += 3;

    }

    int pkIndex = find(attrs.begin(), attrs.end(), pk) - attrs.begin();
    if (updates.count(pkIndex)) {

        if (find(T.begin() + pSet, T.end(), "where") == T.end()) {
            cout << "[SaadDB] Refuse updating PK without WHERE.\n"; return;
        }
    }

    ifstream in(table + ".sdb");
    if (!in) { cout << "[SaadDB] No data.\n"; return; }
    ofstream out("tmp.sdb");
    string line; int affected = 0;
    while (safe_getline(in, line)) {
        if (line.empty()) { out << "\n"; continue; }
        auto row = split_csv_inside_tuple(line);
        if (row.size() != attrs.size()) { out << line << "\n"; continue; }
        if (eval_where(row, attrs, T, 2)) { 

            for (auto& kv : updates) row[kv.first] = kv.second;
            ++affected;
        }
        out << join_csv_tuple(row) << "\n";
    }
    in.close(); out.close();
    remove((table + ".sdb").c_str());
    rename("tmp.sdb", (table + ".sdb").c_str());
    cout << "[SaadDB] " << affected << " rows affected.\n";
}

static void cmd_delete(const vector<string>& T) {

    if (T.size() < 3 || T[0] != "delete" || T[1] != "from") { cout << "[SaadDB] INVALID DELETE\n"; return; }
    string table = T[2];
    if (!ensure_table_exists(table)) return;

    ifstream in(table + ".sdb");
    if (!in) { cout << "[SaadDB] 0 rows affected.\n"; return; }
    ofstream out("tmp.sdb");
    vector<string> attrs; fill_attrs_of(table, attrs);
    string line; int kept = 0, total = 0;
    while (safe_getline(in, line)) {
        if (line.empty()) { out << "\n"; continue; }
        auto row = split_csv_inside_tuple(line);
        if (row.size() != attrs.size()) { out << line << "\n"; continue; }
        ++total;
        if (!eval_where(row, attrs, T, 2)) { out << join_csv_tuple(row) << "\n"; ++kept; }
    }
    in.close(); out.close();
    remove((table + ".sdb").c_str());
    rename("tmp.sdb", (table + ".sdb").c_str());
    cout << "[SaadDB] " << (total - kept) << " rows affected.\n";
}

// ---------- executor ----------
static void execute() {
    if (TOKENS.empty()) return;
    const string& t0 = TOKENS[0];
    if (t0 == "help")          return cmd_help(TOKENS);
    if (t0 == "create")        return cmd_create(TOKENS);
    if (t0 == "drop")          return cmd_drop(TOKENS);
    if (t0 == "describe")      return cmd_describe(TOKENS);
    if (t0 == "insert")        return cmd_insert(TOKENS);
    if (t0 == "select")        return cmd_select(TOKENS);
    if (t0 == "update")        return cmd_update(TOKENS);
    if (t0 == "delete")        return cmd_delete(TOKENS);
    if (t0 == "quit") { cout << "[SaadDB] Bye.\n"; exit(0); }
    cout << "[SaadDB] INVALID QUERY\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "=== Saad DB (C++ mini-SQL) ===\n";
    cout << "Type help; or quit;\n";
    string q;
    while (true) {
        cout << "\n>> ";
        if (!std::getline(cin, q)) break;
        if (!has_semicolon(q)) { cout << "[SaadDB] ; missing at the end\n"; continue; }
        parse_tokens(q);
        execute();
    }
    return 0;
}
