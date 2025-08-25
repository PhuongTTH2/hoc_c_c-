using NUnit.Framework;
using MathLib;

namespace MathLibTests
{
    public class MathUtilsTests
    {
        [Test]
        public void Add_TwoNumbers_ReturnsSum()
        {
            Assert.AreEqual(5, MathUtils.Add(2, 3));
            Assert.AreEqual(0, MathUtils.Add(-1, 1));
            Assert.AreEqual(0, MathUtils.Add(0, 0));
        }
    }
}