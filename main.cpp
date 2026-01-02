#include <iostream>
#include <string>
#include <limits>

#include "storage/storage.h"
#include "auth/auth.h"

static constexpr const char* ADMIN_PASSWORD = "admin123";

// ===== 用户登录后的菜单 =====
static void logged_in_menu(AuthService& auth,
                           UserStorage& storage,
                           const std::string& username) {
    while (true) {
        std::string err;
        auto uopt = storage.find_by_username(username, err);
        if (!err.empty()) {
            std::cout << "[ERR] " << err << "\n";
            return;
        }
        if (!uopt) {
            std::cout << "[ERR] User not found.\n";
            return;
        }

        const auto& u = *uopt;

        std::cout << "\n== User Panel ==\n";
        std::cout << "uid: " << u.uid << "\n";
        std::cout << "username: " << u.username << "\n";
        std::cout << "nickname: " << u.nickname << "\n";
        std::cout << "goal: " << u.goal << "\n";
        std::cout << "location: " << u.location << "\n";
        std::cout << "time: " << u.time << "\n\n";

        std::cout << "1) Edit nickname\n";
        std::cout << "2) Edit study info\n";
        std::cout << "3) Change password\n";
        std::cout << "4) Logout\n";
        std::cout << "> ";

        int op;
        if (!(std::cin >> op)) return;

        if (op == 4) return;

        if (op == 1) {
            std::cout << "new nickname: ";
            std::string nn;
            std::cin >> nn;
            auto r = auth.update_nickname(username, nn);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        } else if (op == 2) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::string goal, location, time;

            std::cout << "goal: ";
            std::getline(std::cin, goal);
            std::cout << "location: ";
            std::getline(std::cin, location);
            std::cout << "time: ";
            std::getline(std::cin, time);

            auto r = auth.update_study_info(username, goal, location, time);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        } else if (op == 3) {
            std::cout << "new password: ";
            std::string pw;
            std::cin >> pw;
            auto r = auth.reset_password(username, pw);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        }
    }
}

// ===== 管理员面板 =====
static void admin_panel(AuthService& auth) {
    while (true) {
        std::cout << "\n== Admin Panel ==\n";
        std::cout << "1) Delete user\n";
        std::cout << "2) Reset user password\n";
        std::cout << "3) Back\n";
        std::cout << "> ";

        int op;
        if (!(std::cin >> op)) return;
        if (op == 3) return;

        if (op == 1) {
            std::string u;
            std::cout << "username to delete: ";
            std::cin >> u;
            auto r = auth.delete_user(u);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        } else if (op == 2) {
            std::string u, pw;
            std::cout << "username: ";
            std::cin >> u;
            std::cout << "new password: ";
            std::cin >> pw;
            auto r = auth.reset_password(u, pw);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        }
    }
}

int main() {
    UserStorage storage("data/users.jsonl");
    AuthService auth(storage);

    bool is_admin = false;

    while (true) {
        std::cout << "\n== Main Menu ==\n";
        std::cout << "1) Register\n";
        std::cout << "2) Login\n";
        std::cout << "3) Admin login\n";
        std::cout << "4) Admin panel\n";
        std::cout << "5) Admin logout\n";
        std::cout << "6) Exit\n";
        std::cout << "> ";

        int op;
        if (!(std::cin >> op)) break;
        if (op == 6) break;

        if (op == 1) {
            std::string u, p;
            std::cout << "username: ";
            std::cin >> u;
            std::cout << "password: ";
            std::cin >> p;
            auto r = auth.register_user(u, p);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        } else if (op == 2) {
            std::string u, p;
            std::cout << "username: ";
            std::cin >> u;
            std::cout << "password: ";
            std::cin >> p;
            auto r = auth.login_user(u, p);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
            if (r.ok) logged_in_menu(auth, storage, u);
        } else if (op == 3) {
            std::cout << "admin password: ";
            std::string pw;
            std::cin >> pw;
            if (pw == ADMIN_PASSWORD) {
                is_admin = true;
                std::cout << "[OK] Admin logged in.\n";
            } else {
                std::cout << "[ERR] Wrong admin password.\n";
            }
        } else if (op == 4) {
            if (!is_admin) {
                std::cout << "[ERR] Admin not logged in.\n";
            } else {
                admin_panel(auth);
            }
        } else if (op == 5) {
            is_admin = false;
            std::cout << "[OK] Admin logged out.\n";
        }
    }
    return 0;
}
