import os

folder = "graphs/"

for filename in os.listdir(folder):
	if filename.endswith(".dot"):
		filename = os.path.join(folder,filename)
		graphName = filename[:-4] + ".ps"
		os.system("dot -Tps " + filename + " -o " + graphName)