#!/usr/bin/env python3
"""
Defender II Vita Port Legal Patcher
Includes two modes:
1. Developer Mode: Generates a legal Base VPK and a Patch Folder (diffs) from a working VPK + APK.
2. Player Mode: Combines a legal APK, the Base VPK, and the Patch Folder into a playable game.
"""

import os
import shutil
import zipfile
import tempfile
import filecmp
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

# -------------------------------------------------------------------
# Core Logic: Developer (Generate Distributables)
# -------------------------------------------------------------------
def generate_distributables(working_vpk, original_apk, output_dir):
    """
    Compares a working VPK and the original APK.
    Creates a Base VPK (no copyright assets) and a Patch Folder (custom dev files).
    """
    temp_dir = tempfile.mkdtemp(prefix="dev_vpk_")
    vpk_extract = os.path.join(temp_dir, "vpk")
    apk_assets = os.path.join(temp_dir, "apk_assets")
    
    os.makedirs(vpk_extract, exist_ok=True)
    os.makedirs(apk_assets, exist_ok=True)
    
    patch_out_dir = os.path.join(output_dir, "Patch_Folder")
    base_vpk_out = os.path.join(output_dir, "Base.vpk")
    
    try:
        # 1. Extract Working VPK
        with zipfile.ZipFile(working_vpk, 'r') as zf:
            zf.extractall(vpk_extract)
            
        # 2. Extract APK Assets and Map res/raw -> assets/audio
        with zipfile.ZipFile(original_apk, 'r') as zf:
            for info in zf.infolist():
                if info.is_dir(): continue
                
                filename = info.filename
                if filename.startswith('assets/'):
                    out_path = os.path.join(apk_assets, filename)
                    os.makedirs(os.path.dirname(out_path), exist_ok=True)
                    with open(out_path, 'wb') as f: f.write(zf.read(filename))
                elif filename.startswith('res/raw/'):
                    rel_name = filename[len('res/raw/'):]
                    out_path = os.path.join(apk_assets, 'assets', 'audio', rel_name)
                    os.makedirs(os.path.dirname(out_path), exist_ok=True)
                    with open(out_path, 'wb') as f: f.write(zf.read(filename))

        # 3. Compare VPK assets with APK assets to build Patch Folder
        vpk_assets_dir = os.path.join(vpk_extract, 'assets')
        if os.path.exists(vpk_assets_dir):
            for root, _, files in os.walk(vpk_assets_dir):
                for f in files:
                    vpk_file_path = os.path.join(root, f)
                    rel_path = os.path.relpath(vpk_file_path, vpk_assets_dir)
                    apk_file_path = os.path.join(apk_assets, 'assets', rel_path)
                    
                    # If file doesn't exist in APK or is modified by Dev -> Copy to Patch Folder
                    if not os.path.exists(apk_file_path) or not filecmp.cmp(vpk_file_path, apk_file_path, shallow=False):
                        patch_dest = os.path.join(patch_out_dir, 'assets', rel_path)
                        os.makedirs(os.path.dirname(patch_dest), exist_ok=True)
                        shutil.copy2(vpk_file_path, patch_dest)
                        
        # 4. Build Base.vpk (everything EXCEPT the assets folder and any extraneous vpk files)
        with zipfile.ZipFile(base_vpk_out, 'w', compression=zipfile.ZIP_DEFLATED) as zf:
            for root, _, files in os.walk(vpk_extract):
                for f in files:
                    if f.lower().endswith('.vpk'): 
                        continue  # Failsafe: never pack a vpk inside a vpk
                        
                    full_path = os.path.join(root, f)
                    rel_path = os.path.relpath(full_path, vpk_extract).replace(os.sep, '/')
                    if not rel_path.startswith('assets/'):
                        zf.write(full_path, rel_path)

    finally:
        shutil.rmtree(temp_dir)
        
    return base_vpk_out, patch_out_dir

# -------------------------------------------------------------------
# Core Logic: Player (Build Final Game)
# -------------------------------------------------------------------
def build_player_game(legal_apk, base_vpk, patch_folder, output_vpk):
    """
    Extracts base VPK, overlays APK mapped assets, overwrites with Dev's patch folder,
    and packages it all into the final playable VPK.
    """
    temp_dir = tempfile.mkdtemp(prefix="player_build_")
    try:
        # 1. Extract Base VPK
        with zipfile.ZipFile(base_vpk, 'r') as zf:
            zf.extractall(temp_dir)
            
        # 2. Extract APK Assets directly into build dir
        with zipfile.ZipFile(legal_apk, 'r') as zf:
            for info in zf.infolist():
                if info.is_dir(): continue
                
                filename = info.filename
                if filename.startswith('assets/'):
                    out_path = os.path.join(temp_dir, filename)
                    os.makedirs(os.path.dirname(out_path), exist_ok=True)
                    with open(out_path, 'wb') as f: f.write(zf.read(filename))
                elif filename.startswith('res/raw/'):
                    rel_name = filename[len('res/raw/'):]
                    out_path = os.path.join(temp_dir, 'assets', 'audio', rel_name)
                    os.makedirs(os.path.dirname(out_path), exist_ok=True)
                    with open(out_path, 'wb') as f: f.write(zf.read(filename))
                    
        # 3. Apply Patch Folder with Auto-Correction
        actual_patch_folder = patch_folder
        # Check if the user selected the parent directory containing the 'Patch_Folder'
        if os.path.basename(patch_folder) != "Patch_Folder" and os.path.isdir(os.path.join(patch_folder, "Patch_Folder")):
            actual_patch_folder = os.path.join(patch_folder, "Patch_Folder")
            
        if os.path.exists(actual_patch_folder):
            for item in os.listdir(actual_patch_folder):
                if item.lower().endswith('.vpk'):
                    continue # Strict rule: do not copy extraneous .vpk files
                    
                s = os.path.join(actual_patch_folder, item)
                d = os.path.join(temp_dir, item)
                if os.path.isdir(s):
                    shutil.copytree(s, d, dirs_exist_ok=True)
                else:
                    shutil.copy2(s, d)
            
        # 4. Build Final VPK
        with zipfile.ZipFile(output_vpk, 'w', compression=zipfile.ZIP_DEFLATED) as zf:
            for root, _, files in os.walk(temp_dir):
                for f in files:
                    if f.lower().endswith('.vpk'):
                        continue # Final absolute failsafe
                        
                    full_path = os.path.join(root, f)
                    rel_path = os.path.relpath(full_path, temp_dir).replace(os.sep, '/')
                    zf.write(full_path, rel_path)
    finally:
        shutil.rmtree(temp_dir)

# -------------------------------------------------------------------
# GUI Application
# -------------------------------------------------------------------
class VitaPortTool(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Vita Port Legal Distributor & Patcher")
        self.geometry("650x380")
        self.resizable(False, False)
        
        self.notebook = ttk.Notebook(self)
        self.notebook.pack(expand=True, fill='both')
        
        self.dev_tab = ttk.Frame(self.notebook)
        self.player_tab = ttk.Frame(self.notebook)
        
        self.notebook.add(self.player_tab, text='Player Mode (Build Game)')
        self.notebook.add(self.dev_tab, text='Developer Mode (Create Distributables)')
        
        self.setup_player_tab()
        self.setup_dev_tab()

    def setup_player_tab(self):
        frm = tk.Frame(self.player_tab, padx=20, pady=20)
        frm.pack(fill='both', expand=True)
        
        tk.Label(frm, text="1. Legal .apk:").grid(row=0, column=0, sticky="e", pady=8)
        self.pl_apk_var = tk.StringVar()
        tk.Entry(frm, textvariable=self.pl_apk_var, width=45).grid(row=0, column=1, padx=5)
        tk.Button(frm, text="Browse", command=lambda: self.browse_file(self.pl_apk_var, "*.apk")).grid(row=0, column=2)

        tk.Label(frm, text="2. Base .vpk:").grid(row=1, column=0, sticky="e", pady=8)
        self.pl_base_var = tk.StringVar()
        tk.Entry(frm, textvariable=self.pl_base_var, width=45).grid(row=1, column=1, padx=5)
        tk.Button(frm, text="Browse", command=lambda: self.browse_file(self.pl_base_var, "*.vpk")).grid(row=1, column=2)

        tk.Label(frm, text="3. Patch Folder:").grid(row=2, column=0, sticky="e", pady=8)
        self.pl_patch_var = tk.StringVar()
        tk.Entry(frm, textvariable=self.pl_patch_var, width=45).grid(row=2, column=1, padx=5)
        tk.Button(frm, text="Browse", command=lambda: self.browse_dir(self.pl_patch_var)).grid(row=2, column=2)

        tk.Label(frm, text="Save Final VPK as:").grid(row=3, column=0, sticky="e", pady=8)
        self.pl_out_var = tk.StringVar(value="Defender-II-Playable.vpk")
        tk.Entry(frm, textvariable=self.pl_out_var, width=45).grid(row=3, column=1, padx=5)
        tk.Button(frm, text="Browse", command=lambda: self.save_file(self.pl_out_var, "*.vpk")).grid(row=3, column=2)

        tk.Button(frm, text="Build Playable Game", bg="#2196F3", fg="white", font=('Helvetica', 10, 'bold'),
                  command=self.run_player_build).grid(row=4, column=1, pady=20)

    def setup_dev_tab(self):
        frm = tk.Frame(self.dev_tab, padx=20, pady=20)
        frm.pack(fill='both', expand=True)
        
        tk.Label(frm, text="Working Game .vpk:").grid(row=0, column=0, sticky="e", pady=10)
        self.dev_vpk_var = tk.StringVar()
        tk.Entry(frm, textvariable=self.dev_vpk_var, width=45).grid(row=0, column=1, padx=5)
        tk.Button(frm, text="Browse", command=lambda: self.browse_file(self.dev_vpk_var, "*.vpk")).grid(row=0, column=2)

        tk.Label(frm, text="Original Clean .apk:").grid(row=1, column=0, sticky="e", pady=10)
        self.dev_apk_var = tk.StringVar()
        tk.Entry(frm, textvariable=self.dev_apk_var, width=45).grid(row=1, column=1, padx=5)
        tk.Button(frm, text="Browse", command=lambda: self.browse_file(self.dev_apk_var, "*.apk")).grid(row=1, column=2)

        tk.Label(frm, text="Output Directory:").grid(row=2, column=0, sticky="e", pady=10)
        self.dev_out_var = tk.StringVar()
        tk.Entry(frm, textvariable=self.dev_out_var, width=45).grid(row=2, column=1, padx=5)
        tk.Button(frm, text="Browse", command=lambda: self.browse_dir(self.dev_out_var)).grid(row=2, column=2)

        tk.Button(frm, text="Generate Distributables", bg="#4CAF50", fg="white", font=('Helvetica', 10, 'bold'),
                  command=self.run_dev_generate).grid(row=3, column=1, pady=20)

    def browse_file(self, var, ext):
        path = filedialog.askopenfilename(filetypes=[(f"{ext} files", ext)])
        if path: var.set(path)
        
    def browse_dir(self, var):
        path = filedialog.askdirectory()
        if path: var.set(path)
        
    def save_file(self, var, ext):
        path = filedialog.asksaveasfilename(defaultextension=ext.replace('*',''), filetypes=[(f"{ext} files", ext)])
        if path: var.set(path)

    def run_dev_generate(self):
        vpk = self.dev_vpk_var.get()
        apk = self.dev_apk_var.get()
        out = self.dev_out_var.get()
        
        if not (os.path.isfile(vpk) and os.path.isfile(apk) and os.path.isdir(out)):
            messagebox.showerror("Error", "Please provide valid VPK, APK, and Output Directory paths.")
            return
            
        try:
            base, patch = generate_distributables(vpk, apk, out)
            messagebox.showinfo("Success", f"Distributables generated successfully!\n\nReady for release:\n1. {base}\n2. {patch}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to generate distributables:\n{str(e)}")

    def run_player_build(self):
        apk = self.pl_apk_var.get()
        base = self.pl_base_var.get()
        patch = self.pl_patch_var.get()
        out = self.pl_out_var.get()
        
        if not (os.path.isfile(apk) and os.path.isfile(base) and os.path.isdir(patch) and out):
            messagebox.showerror("Error", "Please provide all required files/folders for the build.")
            return
            
        try:
            build_player_game(apk, base, patch, out)
            messagebox.showinfo("Success", f"Game built successfully!\n\nSaved to: {out}\nReady to install via VitaShell.")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to build playable game:\n{str(e)}")

if __name__ == "__main__":
    app = VitaPortTool()
    app.mainloop()