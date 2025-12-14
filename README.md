# Hospital Management System - Quick Reference

## 📋 MỤC LỤC

- [Git Workflow](#git-workflow)
- [Build & Run](#build--run)
- [Troubleshooting](#troubleshooting)
- [Branch Management](#branch-management)

---

## 🔄 GIT WORKFLOW

### Tạo branch mới
```bash
git checkout -b HN28-01
```

### Kiểm tra trạng thái
```bash
git status
```

### Stage changes
```bash
# Stage tất cả files
git add .

# Stage file cụ thể
git add src/Doctor.cpp
```

### Commit
```bash
git commit -m "Add Doctor class with basic functions"
```

### Push lên remote
```bash
# Push branch mới lần đầu
git push origin HN28-01

# Push tiếp cho branch đã có
git push
```

### Pull code mới từ main
```bash
# Checkout về main
git checkout main

# Pull code mới
git pull origin main
```

### Merge main vào branch đang làm
```bash
# Đang ở branch của bạn
git checkout HN28-01

# Merge main vào
git merge main

# Nếu có conflict, resolve xong rồi:
git add .
git commit -m "Resolve merge conflicts"
```

### Xem lịch sử commit
```bash
# Xem ngắn gọn
git log --oneline

# Xem chi tiết
git log

# Xem dạng graph
git log --oneline --graph --all
```

---

## 🏗️ BUILD & RUN

### Build toàn bộ project
```bash
cd build
cmake ..
make
```

### Build riêng HospitalApp
```bash
cd build
make HospitalApp
./HospitalApp
```

### Build riêng HospitalTests
```bash
cd build
make HospitalTests
./HospitalTests
```

### Build và chạy trong 1 lệnh
```bash
# Build và chạy HospitalApp
cd build && make HospitalApp && ./HospitalApp

# Build và chạy HospitalTests
cd build && make HospitalTests && ./HospitalTests
```

### Clean build (xóa files build cũ)
```bash
cd build
make clean
cmake ..
make
```

### Rebuild từ đầu
```bash
rm -rf build
mkdir build
cd build
cmake ..
make
```

---

## 🔧 TROUBLESHOOTING

### Lỡ commit vào main local

**Vấn đề:** Đã commit vào main nhưng chưa push được (vì branch protection)

**Giải pháp:**
```bash
# Tạo branch mới từ vị trí hiện tại
git checkout -b HN28-01

# Quay lại main và reset
git checkout main
git reset --hard origin/main

# Push branch mới
git checkout HN28-01
git push origin HN28-01
```

### Xóa branch sau khi merge

```bash
# Cập nhật danh sách remote
git fetch --prune

# Checkout về main
git checkout main

# Xóa branch local
git branch -d HN28-01

# Nếu Git từ chối xóa, dùng -D
git branch -D HN28-01
```

### Không thể push vì remote có code mới

**Lỗi:** `Can't push refs to remote. Try running Pull first`

**Giải pháp:**
```bash
# Pull code về trước
git pull origin main

# Nếu có conflict, resolve xong rồi:
git add .
git commit -m "Resolve conflicts"

# Push lại
git push origin main
```

### Undo commit gần nhất (giữ changes)

```bash
# Undo commit, giữ changes ở staged
git reset --soft HEAD~1

# Undo commit, giữ changes ở unstaged
git reset HEAD~1

# Undo commit, XÓA HẾT changes (NGUY HIỂM!)
git reset --hard HEAD~1
```

### Stash changes (cất code tạm thời)

```bash
# Cất code đang làm dở
git stash

# Xem danh sách stash
git stash list

# Lấy lại code
git stash pop

# Lấy lại nhưng giữ stash
git stash apply
```

### Xem thay đổi trước khi commit

```bash
# Xem changes chưa staged
git diff

# Xem changes đã staged
git diff --staged

# Xem changes của file cụ thể
git diff src/Doctor.cpp
```

---

## 🌿 BRANCH MANAGEMENT

### Xem danh sách branches

```bash
# Xem local branches
git branch

# Xem tất cả (local + remote)
git branch -a

# Xem remote branches
git branch -r
```

### Chuyển branch

```bash
git checkout HN28-01

# Hoặc dùng switch (Git 2.23+)
git switch HN28-01
```

### Tạo và chuyển sang branch mới

```bash
git checkout -b HN28-02

# Hoặc dùng switch
git switch -c HN28-02
```

### Xóa branch local

```bash
# Xóa branch đã merge
git branch -d HN28-01

# Force delete (chưa merge)
git branch -D HN28-01
```

### Xóa branch remote

```bash
git push origin --delete HN28-01
```

### Đổi tên branch

```bash
# Đổi tên branch hiện tại
git branch -m HN28-new-name

# Đổi tên branch khác
git branch -m old-name new-name
```

---

## 🔍 FETCH & PULL

### Fetch (kiểm tra code mới, không tải về)

```bash
# Fetch từ origin
git fetch

# Fetch và xóa remote branches đã bị xóa
git fetch --prune
git fetch -p
```

### Pull (tải code về và merge)

```bash
# Pull từ main
git pull origin main

# Pull với rebase thay vì merge
git pull --rebase origin main
```

---

## 📝 COMMIT BEST PRACTICES

### Good commit messages

✅ **TốT:**
```bash
git commit -m "Add Doctor class with basic methods"
git commit -m "Fix bug in Patient registration form"
git commit -m "Update README with build instructions"
```

❌ **KHÔNG TỐT:**
```bash
git commit -m "update"
git commit -m "fix"
git commit -m "aaa"
```

### Commit template

```bash
# Feature
git commit -m "Add <what>"

# Bug fix
git commit -m "Fix <issue>"

# Update
git commit -m "Update <what> to <reason>"

# Refactor
git commit -m "Refactor <what> for <reason>"
```

---

## 🚨 QUAN TRỌNG

### ⚠️ KHÔNG BAO GIỜ:

- ❌ Code trực tiếp trên main
- ❌ Force push lên main (`git push -f`)
- ❌ Xóa .devcontainer, .github, CMakeLists.txt
- ❌ Pull khi code đang làm dở chưa commit

### ✅ LUÔN LUÔN:

- ✅ Tạo branch mới cho mỗi tính năng
- ✅ Commit thường xuyên với message rõ ràng
- ✅ Pull main về trước khi bắt đầu làm việc
- ✅ Test code trước khi push

---

## 📞 HỖ TRỢ

Nếu gặp vấn đề:

1. Kiểm tra lại các lệnh trong file này
2. Xem file `Cam_nang_lam_viec_Codespaces.docx`
3. Hỏi nhóm trưởng hoặc thành viên khác

---

## 🔗 LIÊN KẾT HỮU ÍCH

- [Git Documentation](https://git-scm.com/doc)
- [GitHub Guides](https://guides.github.com/)
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

---

**Cập nhật lần cuối:** $(date +"%d/%m/%Y")