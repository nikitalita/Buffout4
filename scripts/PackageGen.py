import argparse
import os
import zipfile
import zlib

def make_rel_archive(a_args):
	archive = zipfile.ZipFile(a_args.name + ".zip", "w", zipfile.ZIP_DEFLATED)
	def do_write(a_path, a_ext):
		archive.write(a_path, "F4SE/Plugins/" + a_args.name + a_ext)

	do_write(a_args.dll, ".dll")
	do_write(a_args.src_dir + "/Buffout4.toml", ".toml")
	do_write(a_args.src_dir + "/Buffout4_preload.txt", "_preload.txt")

def make_dbg_archive(a_args):
	archive = zipfile.ZipFile(a_args.name + "_pdb" + ".zip", "w", zipfile.ZIP_DEFLATED)
	archive.write(a_args.pdb, a_args.name + ".pdb")

def parse_arguments():
	parser = argparse.ArgumentParser(description="archive build artifacts for distribution")
	parser.add_argument("--name", type=str, help="the project name", required=True)
	parser.add_argument("--src-dir", type=str, help="the project root source directory", required=True)
	parser.add_argument("--dll", type=str, help="the full dll path", required=True)
	parser.add_argument("--pdb", type=str, help="the full pdb path", required=True)
	return parser.parse_args()

def main():
	out = "artifacts"
	try:
		os.mkdir(out)
	except FileExistsError:
		pass
	os.chdir(out)

	args = parse_arguments()
	make_rel_archive(args)
	make_dbg_archive(args)

if __name__ == "__main__":
	main()
