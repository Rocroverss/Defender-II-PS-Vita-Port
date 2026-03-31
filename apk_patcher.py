import os
import zipfile
import shutil
import filecmp
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext, ttk

# ------------------------
# Core logic (same as before)
# ------------------------

def extract_zip(zip_path, out_dir):
    with zipfile.ZipFile(zip_path, 'r') as z:
        z.extractall(out_dir)

def create_zip(folder, out_file):
    with zipfile.ZipFile(out_file, 'w', zipfile.ZIP_DEFLATED) as z:
        for root, _, files in os.walk(folder):
            for f in files:
                full = os.path.join(root, f)
                rel = os.path.relpath(full, folder)
                z.write(full, rel)

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

def files_different(f1, f2):
    if not os.path.exists(f2):
        return True
    return not filecmp.cmp(f1, f2, shallow=False)

# ------------------------
# PATCH GENERATOR
# ------------------------

def generate_patch(apk_path, modified_assets_dir, output_patch_dir, log):
    temp_apk = "tmp_apk"

    log("[*] Extracting APK...")
    extract_zip(apk_path, temp_apk)

    apk_assets = os.path.join(temp_apk, "assets")
    apk_audio = os.path.join(temp_apk, "res", "raw")

    patch_assets = os.path.join(output_patch_dir, "assets")
    patch_audio = os.path.join(patch_assets, "audio")

    ensure_dir(patch_assets)
    ensure_dir(patch_audio)

    log("[*] Comparing assets...")

    for root, _, files in os.walk(os.path.join(modified_assets_dir, "assets")):
        for f in files:
            mod_file = os.path.join(root, f)
            rel = os.path.relpath(mod_file, os.path.join(modified_assets_dir, "assets"))

            apk_file = os.path.join(apk_assets, rel)
            out_file = os.path.join(patch_assets, rel)

            if files_different(mod_file, apk_file):
                ensure_dir(os.path.dirname(out_file))
                shutil.copy2(mod_file, out_file)
                log(f"[+] Asset: {rel}")

    audio_src = os.path.join(modified_assets_dir, "assets", "audio")

    if os.path.exists(audio_src):
        for root, _, files in os.walk(audio_src):
            for f in files:
                mod_file = os.path.join(root, f)
                rel = os.path.relpath(mod_file, audio_src)

                apk_file = os.path.join(apk_audio, rel)
                out_file = os.path.join(patch_audio, rel)

                if files_different(mod_file, apk_file):
                    ensure_dir(os.path.dirname(out_file))
                    shutil.copy2(mod_file, out_file)
                    log(f"[+] Audio: {rel}")

    shutil.rmtree(temp_apk)
    log("[✓] Patch generated!")

# ------------------------
# PATCH APPLIER
# ------------------------

def apply_patch(apk_path, patch_dir, vpk_out, log):
    temp_apk = "tmp_apk"
    temp_build = "tmp_build"

    log("[*] Extracting APK...")
    extract_zip(apk_path, temp_apk)

    assets_dir = os.path.join(temp_apk, "assets")
    raw_dir = os.path.join(temp_apk, "res", "raw")

    patch_assets = os.path.join(patch_dir, "assets")

    log("[*] Applying assets...")
    for root, _, files in os.walk(patch_assets):
        for f in files:
            src = os.path.join(root, f)
            rel = os.path.relpath(src, patch_assets)

            if rel.startswith("audio"):
                continue

            dst = os.path.join(assets_dir, rel)
            ensure_dir(os.path.dirname(dst))
            shutil.copy2(src, dst)

    patch_audio = os.path.join(patch_assets, "audio")

    if os.path.exists(patch_audio):
        log("[*] Applying audio...")
        for root, _, files in os.walk(patch_audio):
            for f in files:
                src = os.path.join(root, f)
                rel = os.path.relpath(src, patch_audio)

                dst = os.path.join(raw_dir, rel)
                ensure_dir(os.path.dirname(dst))
                shutil.copy2(src, dst)

    log("[*] Building VPK...")
    vpk_root = os.path.join(temp_build, "app", "DEEF0001")
    ensure_dir(vpk_root)

    shutil.copytree(assets_dir, os.path.join(vpk_root, "assets"))

    if os.path.exists("sce_sys"):
        shutil.copytree("sce_sys", os.path.join(vpk_root, "sce_sys"))

    if os.path.exists("eboot.bin"):
        shutil.copy2("eboot.bin", os.path.join(vpk_root, "eboot.bin"))

    create_zip(temp_build, vpk_out)

    shutil.rmtree(temp_apk)
    shutil.rmtree(temp_build)

    log(f"[✓] VPK created: {vpk_out}")

# ------------------------
# GUI helpers
# ------------------------

def make_logger(console):
    def log(msg):
        console.insert(tk.END, msg + "\n")
        console.see(tk.END)
        console.update()
    return log

def browse_file(entry, filetypes=None, save=False):
    if save:
        path = filedialog.asksaveasfilename(defaultextension=".vpk")
    else:
        path = filedialog.askopenfilename(filetypes=filetypes)
    if path:
        entry.delete(0, tk.END)
        entry.insert(0, path)

def browse_dir(entry):
    path = filedialog.askdirectory()
    if path:
        entry.delete(0, tk.END)
        entry.insert(0, path)

# ------------------------
# GUI
# ------------------------

root = tk.Tk()
root.title("APK ↔ PSVita Tool")
root.geometry("650x500")

tabs = ttk.Notebook(root)
tabs.pack(fill="both", expand=True)

# ========================
# TAB 1: CREATE PATCH
# ========================

tab1 = tk.Frame(tabs)
tabs.add(tab1, text="🛠 Create Patch")

tk.Label(tab1, text="APK").pack()
apk_gen = tk.Entry(tab1, width=60)
apk_gen.pack()
tk.Button(tab1, text="Browse", command=lambda: browse_file(apk_gen, [("APK", "*.apk")])).pack()

tk.Label(tab1, text="Modified Folder").pack()
mod_gen = tk.Entry(tab1, width=60)
mod_gen.pack()
tk.Button(tab1, text="Browse", command=lambda: browse_dir(mod_gen)).pack()

tk.Label(tab1, text="Output Patch Folder").pack()
out_gen = tk.Entry(tab1, width=60)
out_gen.pack()
tk.Button(tab1, text="Browse", command=lambda: browse_dir(out_gen)).pack()

console1 = scrolledtext.ScrolledText(tab1, height=10)
console1.pack(fill="both", pady=5)

log1 = make_logger(console1)

tk.Button(
    tab1,
    text="Generate Patch",
    bg="lightblue",
    command=lambda: generate_patch(apk_gen.get(), mod_gen.get(), out_gen.get(), log1)
).pack(pady=5)

# ========================
# TAB 2: APPLY PATCH
# ========================

tab2 = tk.Frame(tabs)
tabs.add(tab2, text="📦 Apply Patch")

tk.Label(tab2, text="APK").pack()
apk_app = tk.Entry(tab2, width=60)
apk_app.pack()
tk.Button(tab2, text="Browse", command=lambda: browse_file(apk_app, [("APK", "*.apk")])).pack()

tk.Label(tab2, text="Patch Folder").pack()
patch_app = tk.Entry(tab2, width=60)
patch_app.pack()
tk.Button(tab2, text="Browse", command=lambda: browse_dir(patch_app)).pack()

tk.Label(tab2, text="Output VPK").pack()
vpk_app = tk.Entry(tab2, width=60)
vpk_app.pack()
tk.Button(tab2, text="Browse", command=lambda: browse_file(vpk_app, save=True)).pack()

console2 = scrolledtext.ScrolledText(tab2, height=10)
console2.pack(fill="both", pady=5)

log2 = make_logger(console2)

tk.Button(
    tab2,
    text="Apply Patch & Build VPK",
    bg="lightgreen",
    command=lambda: apply_patch(apk_app.get(), patch_app.get(), vpk_app.get(), log2)
).pack(pady=5)

root.mainloop()