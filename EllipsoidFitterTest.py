import numpy as np
import math

np.set_printoptions(formatter={"float_kind": "{:.9f}".format})

x_0 = 3
y_0 = -1
theta_semimajor = math.radians(45)
basis = np.array(
    [
        [np.cos(theta_semimajor), -np.sin(theta_semimajor)],
        [np.sin(theta_semimajor), np.cos(theta_semimajor)],
    ]
)
semiaxes_values = np.array([3 / 4, 5 / 4])
semiaxes = basis.dot(np.diag(semiaxes_values)).dot(basis.T)

N = 32
noise_level = .75
methods = set()

theta = np.random.random(N) * 2 * math.pi
x, y = semiaxes.dot(np.array([np.cos(theta), np.sin(theta)]))
x += (2 * np.random.random(N) - 1) / 2 * noise_level + x_0
y += (2 * np.random.random(N) - 1) / 2 * noise_level + y_0


def basis_theta(basis):
    return math.degrees(math.atan2(basis[1, 0], basis[0, 0]) % math.pi)


def r(x, y, theta):
    c, s = math.cos(theta), math.sin(theta)
    return np.array([x * c - y * s, x * s + y * c])


def sorted_eig(m):
    values, vectors = np.linalg.eig(m)
    i = [*reversed(np.argsort(values))]
    return values.take(i), vectors.take(i, axis=1)


@methods.add
def eig_of_moments(x, y):
    def moments(x, y):
        m = np.array([[1, x, y, x * y, x**2, y**2]])
        return m * m.T

    m = sum([moments(x, y) for x, y in zip(x, y)])
    coeffs = sorted_eig(m)[1].T[-1]

    AK2 = np.array([[coeffs[4], coeffs[3] / 2], [coeffs[3] / 2, coeffs[5]]])
    values, vectors = sorted_eig(AK2)

    AK2_inv = vectors.dot(np.diag([1 / v for v in values])).dot(vectors.T)
    offset = -AK2_inv.dot([coeffs[1], coeffs[2]]) / 2

    K2 = offset.dot(AK2).dot(offset) - coeffs[0]
    A_values = np.array([math.sqrt(v / K2) for v in values])
    return vectors, offset, A_values


@methods.add
def solve_constant_1(x, y):
    def moments(x, y):
        m = np.array([[x, y, x * y, x**2, y**2]])
        return m * m.T

    m = sum([moments(x, y) for x, y in zip(x, y)])
    b = -np.array(
        [
            sum(x),
            sum(y),
            sum(x * y),
            sum(x**2),
            sum(y**2),
        ]
    )
    coeffs = np.linalg.solve(m, b)

    AK2 = np.array([[coeffs[3], coeffs[2] / 2], [coeffs[2] / 2, coeffs[4]]])
    values, vectors = sorted_eig(AK2)

    AK2_inv = vectors.dot(np.diag([1 / v for v in values])).dot(vectors.T)
    offset = -AK2_inv.dot([coeffs[0], coeffs[1]]) / 2

    K2 = offset.dot(AK2).dot(offset) - 1
    A_values = np.array([math.sqrt(v / K2) for v in values])
    return vectors, offset, A_values


@methods.add
def solve_constant_1_symd(x, y):
    def moments(x, y):
        m = np.array([[x, y, x * y, (x**2 + y**2), (x**2 - y**2) / 2]])
        return m * m.T

    m = sum([moments(x, y) for x, y in zip(x, y)])
    b = -np.array(
        [
            sum(x),
            sum(y),
            sum(x * y),
            sum(x**2 + y**2),
            sum(x**2 - y**2) / 2,
        ]
    )
    coeffs = np.linalg.solve(m, b)

    AK2 = np.array(
        [
            [coeffs[3] + coeffs[4] / 2, coeffs[2] / 2],
            [coeffs[2] / 2, coeffs[3] - coeffs[4] / 2],
        ]
    )
    values, vectors = sorted_eig(AK2)

    AK2_inv = vectors.dot(np.diag([1 / v for v in values])).dot(vectors.T)
    offset = -AK2_inv.dot([coeffs[0], coeffs[1]]) / 2

    K2 = offset.dot(AK2).dot(offset) - 1
    A_values = np.array([math.sqrt(v / K2) for v in values])
    return vectors, offset, A_values


@methods.add
def solve_constant_trace(x, y):
    def moments(x, y):
        m = np.array([[1, x, y, x * y, x**2 - y**2]])
        return m * m.T

    m = sum([moments(x, y) for x, y in zip(x, y)])
    b = -np.array([sum(y**2 * m) for m in [1, x, y, x * y, x**2 - y**2]])
    coeffs = np.linalg.solve(m, b)

    AK2 = np.array([[coeffs[4], coeffs[3] / 2], [coeffs[3] / 2, 1 - coeffs[4]]])
    values, vectors = sorted_eig(AK2)

    AK2_inv = vectors.dot(np.diag([1 / v for v in values])).dot(vectors.T)
    offset = -AK2_inv.dot([coeffs[1], coeffs[2]]) / 2

    K2 = offset.dot(AK2).dot(offset) - coeffs[0]
    A_values = np.array([math.sqrt(v / K2) for v in values])
    return vectors, offset, A_values


@methods.add
def solve_constant_trace_symd(x, y):
    def moments(x, y):
        m = np.array([[1, x, y, x * y, x**2 - y**2]])
        return m * m.T

    m = sum([moments(x, y) for x, y in zip(x, y)])
    b = -np.array(
        [sum((x**2 + y**2) * m) for m in [1, x, y, x * y, x**2 - y**2]]
    )
    coeffs = np.linalg.solve(m, b)

    AK2 = np.array([[1 + coeffs[4], coeffs[3] / 2], [coeffs[3] / 2, 1 - coeffs[4]]])
    values, vectors = sorted_eig(AK2)

    AK2_inv = vectors.dot(np.diag([1 / v for v in values])).dot(vectors.T)
    offset = -AK2_inv.dot([coeffs[1], coeffs[2]]) / 2

    K2 = offset.dot(AK2).dot(offset) - coeffs[0]
    A_values = np.array([math.sqrt(v / K2) for v in values])
    return vectors, offset, A_values


print(f"  {basis_theta(basis):.4f} {np.array([x_0, y_0])} {1/semiaxes_values}")
for f in methods:
    print("method:", f.__name__)
    if False:
        # result doesn't depend greatly on rotation angle
        for deg in np.linspace(0, 90, num=5):
            rad = math.radians(deg)
            vectors, offset, A_values = f(*r(x, y, rad))
            theta = basis_theta(vectors) - deg
            offset = r(*offset, -rad)
            print(f"  {theta:.4f} {offset} {A_values}")
    vectors, offset, A_values = f(x, y)
    print(f"  {basis_theta(vectors):.4f} {offset} {A_values}")
    # what are the actual equations when other things do poorly?
    # are others tending toward a linear fit?
    A = vectors.dot(np.diag(A_values)).dot(vectors.T)
    print(f"  {A[0, 0]}x**2 + {A[1, 1]}y**2 + {2*A[0,1]}xy")

print("winner does seem to be constant trace...")
