from matplotlib import pyplot as plt
import numpy as np

data = {
	"double": {
		"Boost": {
			"save": 7.6300,
			"load": 2.7500
		},
		"Cereal": {
			"save": 7.5800,
			"load": 2.7400
		},
		"Ser20": {
			"save": 7.5900,
			"load": 2.7300
		}
	},
	"uint8_t": {
		"Boost": {
			"save": 66.3300,
			"load": 21.4200
		},
		"Cereal": {
			"save": 66.2900,
			"load": 21.4200
		},
		"Ser20": {
			"save": 66.2600,
			"load": 21.4100
		}
	},
	"PoDStruct": {
		"Boost": {
			"save": 102.7800,
			"load": 82.6600
		},
		"Cereal": {
			"save": 75.2500,
			"load": 58.5700
		},
		"Ser20": {
			"save": 37.7900,
			"load": 32.0000
		}
	},
	"PoDChild": {
		"Boost": {
			"save": 66.9400,
			"load": 24.1900
		},
		"Cereal": {
			"save": 68.0000,
			"load": 24.6400
		},
		"Ser20": {
			"save": 67.2900,
			"load": 26.2700
		}
	},
	"String": {
		"Boost": {
			"save": 0.8700,
			"load": 0.7100
		},
		"Cereal": {
			"save": 0.8700,
			"load": 0.7100
		},
		"Ser20": {
			"save": 0.8700,
			"load": 0.7200
		}
	},
	"String": {
		"Boost": {
			"save": 2.1000,
			"load": 3.4200
		},
		"Cereal": {
			"save": 2.5100,
			"load": 4.0700
		},
		"Ser20": {
			"save": 1.3600,
			"load": 3.0900
		}
	},
	"Map(PoDStruct)": {
		"Boost": {
			"save": 5.9300,
			"load": 9.4600
		},
		"Cereal": {
			"save": 4.4800,
			"load": 4.5800
		},
		"Ser20": {
			"save": 2.1400,
			"load": 3.5100
		}
	}
}


fig, axes = plt.subplots(nrows=2, ncols=1)
ax0, ax1 = axes.flatten()

def plot(ax, kind: str):
	X_labels = list(data.keys())
	Y_cereal = [1 for _ in X_labels]
	Y_ser20 = [data[label]["Ser20"][kind] / data[label]["Cereal"][kind] for label in X_labels]
	Y_boost = [data[label]["Boost"][kind] / data[label]["Cereal"][kind] for label in X_labels]
	X_axis = np.arange(len(X_labels))

	ax.bar(X_axis - 0.15, Y_boost, 0.1, label='Boost', color="gray")
	ax.bar(X_axis - 0.05, Y_cereal, 0.1, label='Cereal', color="blue")
	ax.bar(X_axis + 0.05, Y_ser20, 0.1, label='Ser20', color="red")

	ax.set_xticks(X_axis, X_labels)
	ax.set_title(f"Relative performance: {kind}")
	if kind == 'load': ax.legend()

plot(ax0, "load")
plot(ax1, "save")
plt.show()