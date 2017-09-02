-- MySQL dump 10.13  Distrib 5.7.17, for Win64 (x86_64)
--
-- Host: localhost    Database: dbogr
-- ------------------------------------------------------
-- Server version	5.7.19-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `releves`
--

DROP TABLE IF EXISTS `releves`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `releves` (
  `idReleve` bigint(20) NOT NULL AUTO_INCREMENT,
  `vehicule` varchar(255) NOT NULL,
  `w` int(11) DEFAULT NULL,
  `C` double DEFAULT NULL,
  PRIMARY KEY (`idReleve`),
  KEY `fk_Releves_Vehicules_idx` (`vehicule`),
  CONSTRAINT `fk_Releves_Vehicules` FOREIGN KEY (`vehicule`) REFERENCES `vehicules` (`nom`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=265 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `releves`
--

LOCK TABLES `releves` WRITE;
/*!40000 ALTER TABLE `releves` DISABLE KEYS */;
INSERT INTO `releves` VALUES (232,'Motor Sample 001',2000,7.84299408),(233,'Motor Sample 001',2200,7.95018728),(234,'Motor Sample 001',2400,8.030445759),(235,'Motor Sample 001',2600,8.093935651),(236,'Motor Sample 001',2800,8.148906728),(237,'Motor Sample 001',3000,8.201833795),(238,'Motor Sample 001',3200,8.257558077),(239,'Motor Sample 001',3400,8.319428613),(240,'Motor Sample 001',3600,8.389443647),(241,'Motor Sample 001',3800,8.468392021),(242,'Motor Sample 001',4000,8.55599456),(243,'Motor Sample 001',4200,8.651045472),(244,'Motor Sample 001',4400,8.751553731),(245,'Motor Sample 001',4600,8.854884476),(246,'Motor Sample 001',4800,8.957900396),(247,'Motor Sample 001',5000,9.057103125),(248,'Motor Sample 001',5200,9.148774631),(249,'Motor Sample 001',5400,9.229118609),(250,'Motor Sample 001',5600,9.294401872),(251,'Motor Sample 001',5800,9.341095742),(252,'Motor Sample 001',6000,9.36601744),(253,'Motor Sample 001',6200,9.366471481),(254,'Motor Sample 001',6400,9.34039106),(255,'Motor Sample 001',6600,9.286479448),(256,'Motor Sample 001',6800,9.204351383),(257,'Motor Sample 001',7000,9.094674455),(258,'Motor Sample 001',7200,8.959310507),(259,'Motor Sample 001',7400,8.801457018),(260,'Motor Sample 001',7600,8.6257885),(261,'Motor Sample 001',7800,8.438597885),(262,'Motor Sample 001',8000,8.24793792),(263,'Motor Sample 001',8200,8.063762555),(264,'Motor Sample 001',8400,7.898068337);
/*!40000 ALTER TABLE `releves` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `vehicules`
--

DROP TABLE IF EXISTS `vehicules`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicules` (
  `nom` varchar(255) NOT NULL,
  `details` mediumtext,
  `insertDate` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`nom`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `vehicules`
--

LOCK TABLES `vehicules` WRITE;
/*!40000 ALTER TABLE `vehicules` DISABLE KEYS */;
INSERT INTO `vehicules` VALUES ('Motor Sample 001','Exemple de véhicule','2017-08-31 20:13:32');
/*!40000 ALTER TABLE `vehicules` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-09-02 18:29:10
