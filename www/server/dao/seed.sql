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
  `P` double DEFAULT NULL,
  PRIMARY KEY (`idReleve`),
  KEY `fk_Releves_Vehicules_idx` (`vehicule`),
  CONSTRAINT `fk_Releves_Vehicules` FOREIGN KEY (`vehicule`) REFERENCES `vehicules` (`nom`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=67 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `releves`
--

LOCK TABLES `releves` WRITE;
/*!40000 ALTER TABLE `releves` DISABLE KEYS */;
INSERT INTO `releves` VALUES (1,'Motor Sample 001',0,0),(2,'Motor Sample 001',200,0),(3,'Motor Sample 001',400,0),(4,'Motor Sample 001',600,0),(5,'Motor Sample 001',800,0),(6,'Motor Sample 001',1000,0),(7,'Motor Sample 001',1200,0),(8,'Motor Sample 001',1400,0),(9,'Motor Sample 001',1600,0),(10,'Motor Sample 001',1800,0),(11,'Motor Sample 001',2000,211.5645894),(12,'Motor Sample 001',2200,225.1544102),(13,'Motor Sample 001',2400,236.5746697),(14,'Motor Sample 001',2600,246.3173816),(15,'Motor Sample 001',2800,254.8269802),(16,'Motor Sample 001',3000,262.5021006),(17,'Motor Sample 001',3200,269.6973598),(18,'Motor Sample 001',3400,276.7251372),(19,'Motor Sample 001',3600,283.8573556),(20,'Motor Sample 001',3800,291.3272615),(21,'Motor Sample 001',4000,299.3312061),(22,'Motor Sample 001',4200,308.0304261),(23,'Motor Sample 001',4400,317.5528241),(24,'Motor Sample 001',4600,327.9947497),(25,'Motor Sample 001',4800,339.4227799),(26,'Motor Sample 001',5000,351.8755),(27,'Motor Sample 001',5200,365.3652842),(28,'Motor Sample 001',5400,379.8800765),(29,'Motor Sample 001',5600,395.3851713),(30,'Motor Sample 001',5800,411.824994),(31,'Motor Sample 001',6000,429.1248819),(32,'Motor Sample 001',6200,447.192865),(33,'Motor Sample 001',6400,465.9214465),(34,'Motor Sample 001',6600,485.1893835),(35,'Motor Sample 001',6800,504.863468),(36,'Motor Sample 001',7000,524.8003074),(37,'Motor Sample 001',7200,544.8481053),(38,'Motor Sample 001',7400,564.848442),(39,'Motor Sample 001',7600,584.6380557),(40,'Motor Sample 001',7800,604.0506228),(41,'Motor Sample 001',8000,622.9185386),(42,'Motor Sample 001',8200,641.0746984),(43,'Motor Sample 001',8400,658.3542779),(44,'Motor Sample 001',8600,674.596514),(45,'Motor Sample 001',8800,689.6464855),(46,'Motor Sample 001',9000,703.3568941),(47,'Motor Sample 001',9200,715.5898444),(48,'Motor Sample 001',9400,726.2186256),(49,'Motor Sample 001',9600,735.1294913),(50,'Motor Sample 001',9800,742.2234409),(51,'Motor Sample 001',10000,747.418),(52,'Motor Sample 001',10200,750.649001),(53,'Motor Sample 001',10400,751.8723643),(54,'Motor Sample 001',10600,751.0658783),(55,'Motor Sample 001',10800,748.230981),(56,'Motor Sample 001',11000,743.3945399),(57,'Motor Sample 001',11200,736.6106333),(58,'Motor Sample 001',11400,727.9623306),(59,'Motor Sample 001',11600,717.5634733),(60,'Motor Sample 001',11800,705.5604557),(61,'Motor Sample 001',12000,692.1340054),(62,'Motor Sample 001',12200,677.5009644),(63,'Motor Sample 001',12400,0),(64,'Motor Sample 001',12600,0),(65,'Motor Sample 001',12800,0),(66,'Motor Sample 001',13000,0);
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

-- Dump completed on 2017-08-31 20:51:58
